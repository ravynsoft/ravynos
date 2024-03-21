/* isatty() replacement.
   Copyright (C) 2012-2023 Free Software Foundation, Inc.

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

#include <config.h>

/* Specification.  */
#include <unistd.h>

/* This replacement is enabled on native Windows.  */

#include <errno.h>
#include <string.h>

/* Get declarations of the Win32 API functions.  */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#if HAVE_MSVC_INVALID_PARAMETER_HANDLER
# include "msvc-inval.h"
#endif

/* Get _get_osfhandle().  */
#if GNULIB_MSVC_NOTHROW
# include "msvc-nothrow.h"
#else
# include <io.h>
#endif

/* Don't assume that UNICODE is not defined.  */
#undef LoadLibrary
#define LoadLibrary LoadLibraryA
#undef QueryFullProcessImageName
#define QueryFullProcessImageName QueryFullProcessImageNameA

#if !(_WIN32_WINNT >= _WIN32_WINNT_VISTA)

/* Avoid warnings from gcc -Wcast-function-type.  */
# define GetProcAddress \
   (void *) GetProcAddress

/* GetNamedPipeClientProcessId was introduced only in Windows Vista.  */
typedef BOOL (WINAPI * GetNamedPipeClientProcessIdFuncType) (HANDLE hPipe,
                                                             PULONG pClientProcessId);
static GetNamedPipeClientProcessIdFuncType GetNamedPipeClientProcessIdFunc = NULL;
/* QueryFullProcessImageName was introduced only in Windows Vista.  */
typedef BOOL (WINAPI * QueryFullProcessImageNameFuncType) (HANDLE hProcess,
                                                           DWORD dwFlags,
                                                           LPSTR lpExeName,
                                                           PDWORD pdwSize);
static QueryFullProcessImageNameFuncType QueryFullProcessImageNameFunc = NULL;
static BOOL initialized = FALSE;

static void
initialize (void)
{
  HMODULE kernel32 = LoadLibrary ("kernel32.dll");
  if (kernel32 != NULL)
    {
      GetNamedPipeClientProcessIdFunc =
        (GetNamedPipeClientProcessIdFuncType) GetProcAddress (kernel32, "GetNamedPipeClientProcessId");
      QueryFullProcessImageNameFunc =
        (QueryFullProcessImageNameFuncType) GetProcAddress (kernel32, "QueryFullProcessImageNameA");
    }
  initialized = TRUE;
}

#else

# define GetNamedPipeClientProcessIdFunc GetNamedPipeClientProcessId
# define QueryFullProcessImageNameFunc QueryFullProcessImageName

#endif

static BOOL IsConsoleHandle (HANDLE h)
{
  DWORD mode;
  /* GetConsoleMode
     <https://docs.microsoft.com/en-us/windows/console/getconsolemode> */
  return GetConsoleMode (h, &mode) != 0;
}

static BOOL IsCygwinConsoleHandle (HANDLE h)
{
  /* A handle to a Cygwin console is in fact a named pipe whose client process
     and server process is <CYGWIN_INSTALL_DIR>\bin\mintty.exe.  */
  BOOL result = FALSE;
  ULONG processId;

#if !(_WIN32_WINNT >= _WIN32_WINNT_VISTA)
  if (!initialized)
    initialize ();
#endif

  /* GetNamedPipeClientProcessId
     <https://docs.microsoft.com/en-us/windows/desktop/api/winbase/nf-winbase-getnamedpipeclientprocessid>
     It requires -D_WIN32_WINNT=_WIN32_WINNT_VISTA or higher.  */
  if (GetNamedPipeClientProcessIdFunc && QueryFullProcessImageNameFunc
      && GetNamedPipeClientProcessIdFunc (h, &processId))
    {
      /* OpenProcess
         <https://docs.microsoft.com/en-us/windows/desktop/api/processthreadsapi/nf-processthreadsapi-openprocess> */
      HANDLE processHandle =
        OpenProcess (PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);
      if (processHandle != NULL)
        {
          char buf[1024];
          DWORD bufsize = sizeof (buf);
          /* The file name can be determined through
             GetProcessImageFileName
             <https://docs.microsoft.com/en-us/windows/desktop/api/psapi/nf-psapi-getprocessimagefilenamea>
             or
             QueryFullProcessImageName
             <https://docs.microsoft.com/en-us/windows/desktop/api/winbase/nf-winbase-queryfullprocessimagenamea>
             The former returns a file name in non-standard notation (it starts
             with '\Device\') and may require linking with psapi.dll.
             The latter is better, but requires -D_WIN32_WINNT=_WIN32_WINNT_VISTA
             or higher.  */
          if (QueryFullProcessImageNameFunc (processHandle, 0, buf, &bufsize))
            {
              if (strlen (buf) >= 11
                  && strcmp (buf + strlen (buf) - 11, "\\mintty.exe") == 0)
                result = TRUE;
            }
          CloseHandle (processHandle);
        }
    }
  return result;
}

#if HAVE_MSVC_INVALID_PARAMETER_HANDLER
static int
_isatty_nothrow (int fd)
{
  int result;

  TRY_MSVC_INVAL
    {
      result = _isatty (fd);
    }
  CATCH_MSVC_INVAL
    {
      result = 0;
    }
  DONE_MSVC_INVAL;

  return result;
}
#else
# define _isatty_nothrow _isatty
#endif

/* Determine whether FD refers to a console device.  Return 1 if yes.
   Return 0 and set errno if no. (ptsname_r relies on the errno value.)  */
int
isatty (int fd)
{
  HANDLE h = (HANDLE) _get_osfhandle (fd);
  if (h == INVALID_HANDLE_VALUE)
    {
      errno = EBADF;
      return 0;
    }
  /* _isatty (fd) tests whether GetFileType of the handle is FILE_TYPE_CHAR.
     But it does not set errno when it returns 0.  */
  if (_isatty_nothrow (fd))
    {
      if (IsConsoleHandle (h))
        return 1;
    }
  if (IsCygwinConsoleHandle (h))
    return 1;
  errno = ENOTTY;
  return 0;
}
