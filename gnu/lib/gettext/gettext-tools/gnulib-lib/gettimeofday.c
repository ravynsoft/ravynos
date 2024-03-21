/* Provide gettimeofday for systems that don't have it or for which it's broken.

   Copyright (C) 2001-2003, 2005-2007, 2009-2023 Free Software Foundation, Inc.

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

/* written by Jim Meyering */

#include <config.h>

/* Specification.  */
#include <sys/time.h>

#include <time.h>

#if defined _WIN32 && ! defined __CYGWIN__
# define WINDOWS_NATIVE
# include <windows.h>
#endif

#ifdef WINDOWS_NATIVE

/* Don't assume that UNICODE is not defined.  */
# undef LoadLibrary
# define LoadLibrary LoadLibraryA

# if !(_WIN32_WINNT >= _WIN32_WINNT_WIN8)

/* Avoid warnings from gcc -Wcast-function-type.  */
#  define GetProcAddress \
    (void *) GetProcAddress

/* GetSystemTimePreciseAsFileTime was introduced only in Windows 8.  */
typedef void (WINAPI * GetSystemTimePreciseAsFileTimeFuncType) (FILETIME *lpTime);
static GetSystemTimePreciseAsFileTimeFuncType GetSystemTimePreciseAsFileTimeFunc = NULL;
static BOOL initialized = FALSE;

static void
initialize (void)
{
  HMODULE kernel32 = LoadLibrary ("kernel32.dll");
  if (kernel32 != NULL)
    {
      GetSystemTimePreciseAsFileTimeFunc =
        (GetSystemTimePreciseAsFileTimeFuncType) GetProcAddress (kernel32, "GetSystemTimePreciseAsFileTime");
    }
  initialized = TRUE;
}

# else

#  define GetSystemTimePreciseAsFileTimeFunc GetSystemTimePreciseAsFileTime

# endif

#endif

/* This is a wrapper for gettimeofday.  It is used only on systems
   that lack this function, or whose implementation of this function
   causes problems.
   Work around the bug in some systems whereby gettimeofday clobbers
   the static buffer that localtime uses for its return value.  The
   gettimeofday function from Mac OS X 10.0.4 (i.e., Darwin 1.3.7) has
   this problem.  */

int
gettimeofday (struct timeval *restrict tv, void *restrict tz)
{
#undef gettimeofday
#ifdef WINDOWS_NATIVE

  /* On native Windows, there are two ways to get the current time:
     GetSystemTimeAsFileTime
     <https://docs.microsoft.com/en-us/windows/desktop/api/sysinfoapi/nf-sysinfoapi-getsystemtimeasfiletime>
     or
     GetSystemTimePreciseAsFileTime
     <https://docs.microsoft.com/en-us/windows/desktop/api/sysinfoapi/nf-sysinfoapi-getsystemtimepreciseasfiletime>.
     GetSystemTimeAsFileTime produces values that jump by increments of
     15.627 milliseconds (!) on average.
     Whereas GetSystemTimePreciseAsFileTime values usually jump by 1 or 2
     microseconds.
     More discussion on this topic:
     <http://www.windowstimestamp.com/description>.  */
  FILETIME current_time;

# if !(_WIN32_WINNT >= _WIN32_WINNT_WIN8)
  if (!initialized)
    initialize ();
# endif
  if (GetSystemTimePreciseAsFileTimeFunc != NULL)
    GetSystemTimePreciseAsFileTimeFunc (&current_time);
  else
    GetSystemTimeAsFileTime (&current_time);

  /* Convert from FILETIME to 'struct timeval'.  */
  /* FILETIME: <https://docs.microsoft.com/en-us/windows/desktop/api/minwinbase/ns-minwinbase-filetime> */
  ULONGLONG since_1601 =
    ((ULONGLONG) current_time.dwHighDateTime << 32)
    | (ULONGLONG) current_time.dwLowDateTime;
  /* Between 1601-01-01 and 1970-01-01 there were 280 normal years and 89 leap
     years, in total 134774 days.  */
  ULONGLONG since_1970 =
    since_1601 - (ULONGLONG) 134774 * (ULONGLONG) 86400 * (ULONGLONG) 10000000;
  ULONGLONG microseconds_since_1970 = since_1970 / (ULONGLONG) 10;
  *tv = (struct timeval) {
    .tv_sec  = microseconds_since_1970 / (ULONGLONG) 1000000,
    .tv_usec = microseconds_since_1970 % (ULONGLONG) 1000000
  };

  return 0;

#else

# if HAVE_GETTIMEOFDAY

#  if defined timeval /* 'struct timeval' overridden by gnulib?  */
#   undef timeval
  struct timeval otv;
  int result = gettimeofday (&otv, (struct timezone *) tz);
  if (result == 0)
    *tv = otv;
#  else
  int result = gettimeofday (tv, (struct timezone *) tz);
#  endif

  return result;

# else

#  if !defined OK_TO_USE_1S_CLOCK
#   error "Only 1-second nominal clock resolution found.  Is that intended?" \
          "If so, compile with the -DOK_TO_USE_1S_CLOCK option."
#  endif
  *tv = (struct timeval) { .tv_sec = time (NULL), .tv_usec = 0 };

  return 0;

# endif
#endif
}
