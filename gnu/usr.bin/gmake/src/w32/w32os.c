/* Windows32-based operating system interface for GNU Make.
Copyright (C) 2016-2023 Free Software Foundation, Inc.
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

#include <stdio.h>
#include <string.h>

#include <windows.h>
#include <process.h>
#include <io.h>
#if _WIN32_WINNT > 0x0601
#include <synchapi.h>
#endif
#include "pathstuff.h"
#include "sub_proc.h"
#include "w32err.h"
#include "os.h"
#include "debug.h"

unsigned int
check_io_state ()
{
  static unsigned int state = IO_UNKNOWN;

  /* We only need to compute this once per process.  */
  if (state != IO_UNKNOWN)
    return state;

  /* Could have used GetHandleInformation, but that isn't supported
     on Windows 9X.  */
  HANDLE outfd = (HANDLE)_get_osfhandle (fileno (stdout));
  HANDLE errfd = (HANDLE)_get_osfhandle (fileno (stderr));

  if ((HANDLE)_get_osfhandle (fileno (stdin)) != INVALID_HANDLE_VALUE)
    state |= IO_STDIN_OK;
  if (outfd != INVALID_HANDLE_VALUE)
    state |= IO_STDOUT_OK;
  if (errfd != INVALID_HANDLE_VALUE)
    state |= IO_STDERR_OK;

  if (ALL_SET (state, IO_STDOUT_OK|IO_STDERR_OK))
    {
      unsigned int combined = 0;

      if (outfd == errfd)
        combined = IO_COMBINED_OUTERR;
      else
        {
          DWORD outtype = GetFileType (outfd), errtype = GetFileType (errfd);

          if (outtype == errtype
              && outtype != FILE_TYPE_UNKNOWN && errtype != FILE_TYPE_UNKNOWN)
            {
              if (outtype == FILE_TYPE_CHAR)
                {
                  /* For character devices, check if they both refer to a
                     console.  This loses if both handles refer to the
                     null device (FIXME!), but in that case we don't care
                     in the context of Make.  */
                  DWORD outmode, errmode;

                  /* Each process on Windows can have at most 1 console,
                     so if both handles are for the console device, they
                     are the same.  We also compare the console mode to
                     distinguish between stdin and stdout/stderr.  */
                  if (GetConsoleMode (outfd, &outmode)
                      && GetConsoleMode (errfd, &errmode)
                      && outmode == errmode)
                    combined = IO_COMBINED_OUTERR;
                }
              else
                {
                  /* For disk files and pipes, compare their unique
                     attributes.  */
                  BY_HANDLE_FILE_INFORMATION outfi, errfi;

                  /* Pipes get zero in the volume serial number, but do
                     appear to have meaningful information in file index
                     attributes.  We test file attributes as well, for a
                     good measure.  */
                  if (GetFileInformationByHandle (outfd, &outfi)
                      && GetFileInformationByHandle (errfd, &errfi)
                      && outfi.dwVolumeSerialNumber == errfi.dwVolumeSerialNumber
                      && outfi.nFileIndexLow == errfi.nFileIndexLow
                      && outfi.nFileIndexHigh == errfi.nFileIndexHigh
                      && outfi.dwFileAttributes == errfi.dwFileAttributes)
                    combined = IO_COMBINED_OUTERR;
                }
            }
        }
      state |= combined;
    }

  return state;
}

/* A replacement for tmpfile, since the MSVCRT implementation creates
   the file in the root directory of the current drive, which might
   not be writable by our user, and also it returns a FILE* and we want a file
   descriptor.  Mostly borrowed from create_batch_file, see job.c.  */
int
os_anontmp ()
{
  char temp_path[MAX_PATH+1];
  unsigned path_size = GetTempPath (sizeof (temp_path), temp_path);
  int using_cwd = 0;

  /* These variables are static so we won't try to reuse a name that was
     generated a little while ago, because that file might not be on disk yet,
     since we use FILE_ATTRIBUTE_TEMPORARY below, which tells the OS it
     doesn't need to flush the cache to disk.  If the file is not yet on disk,
     we might think the name is available, while it really isn't.  This
     happens in parallel builds.  */
  static unsigned uniq = 0;
  static int second_loop = 0;

  const char base[] = "gmake_tmpf";
  const unsigned sizemax = sizeof (base) - 1 + 4 + 10 + 10;
  unsigned pid = GetCurrentProcessId ();

  if (path_size == 0)
    {
      path_size = GetCurrentDirectory (sizeof (temp_path), temp_path);
      using_cwd = 1;
    }

  ++uniq;
  if (uniq >= 0x10000 && !second_loop)
    {
      /* If we already had 64K batch files in this
         process, make a second loop through the numbers,
         looking for free slots, i.e. files that were
         deleted in the meantime.  */
      second_loop = 1;
      uniq = 1;
    }

  while (path_size > 0 && path_size + sizemax < sizeof (temp_path)
         && (uniq < 0x10000 || !second_loop))
    {
      HANDLE h;

      sprintf (temp_path + path_size,
               "%s%s%u-%x.tmp",
               temp_path[path_size - 1] == '\\' ? "" : "\\",
               base, pid, uniq);
      h = CreateFile (temp_path,  /* file name */
                      GENERIC_READ | GENERIC_WRITE | DELETE, /* desired access */
                      FILE_SHARE_READ | FILE_SHARE_WRITE,    /* share mode */
                      NULL,                                  /* default security attributes */
                      CREATE_NEW,                            /* creation disposition */
                      FILE_ATTRIBUTE_NORMAL |                /* flags and attributes */
                      FILE_ATTRIBUTE_TEMPORARY |
                      FILE_FLAG_DELETE_ON_CLOSE,
                      NULL);                                 /* no template file */

      if (h != INVALID_HANDLE_VALUE)
        return _open_osfhandle ((intptr_t)h, 0);

      {
        const DWORD er = GetLastError ();

        if (er == ERROR_FILE_EXISTS || er == ERROR_ALREADY_EXISTS)
          {
            ++uniq;
            if (uniq == 0x10000 && !second_loop)
              {
                second_loop = 1;
                uniq = 1;
              }
          }
        /* The temporary path is not guaranteed to exist, or might not be
           writable by user.  Use the current directory as fallback.  */
        else if (!using_cwd)
          {
            path_size = GetCurrentDirectory (sizeof (temp_path), temp_path);
            using_cwd = 1;
          }
        else
          {
            errno = EACCES;
            return -1;
          }
      }
    }

  if (uniq >= 0x10000)
    errno = EEXIST;
  return -1;
}

#if defined(MAKE_JOBSERVER)

/* This section provides OS-specific functions to support the jobserver.  */

static char jobserver_semaphore_name[MAX_PATH + 1];
static HANDLE jobserver_semaphore = NULL;

unsigned int
jobserver_setup (int slots, const char *style)
{
  /* sub_proc.c is limited in the number of objects it can wait for. */

  if (style && strcmp (style, "sem") != 0)
    OS (fatal, NILF, _("unknown jobserver auth style '%s'"), style);

  if (slots > process_table_usable_size())
    {
      slots = process_table_usable_size();
      DB (DB_JOBS, (_("jobserver slots limited to %d\n"), slots));
    }

  sprintf (jobserver_semaphore_name, "gmake_semaphore_%d", _getpid ());

  jobserver_semaphore = CreateSemaphore (
      NULL,                           /* Use default security descriptor */
      slots,                          /* Initial count */
      slots,                          /* Maximum count */
      jobserver_semaphore_name);      /* Semaphore name */

  if (jobserver_semaphore == NULL)
    {
      DWORD err = GetLastError ();
      const char *estr = map_windows32_error_to_string (err);
      ONS (fatal, NILF,
           _("creating jobserver semaphore: (Error %ld: %s)"), err, estr);
    }

  return 1;
}

unsigned int
jobserver_parse_auth (const char *auth)
{
  jobserver_semaphore = OpenSemaphore (
      SEMAPHORE_ALL_ACCESS,   /* Semaphore access setting */
      FALSE,                  /* Child processes DON'T inherit */
      auth);                  /* Semaphore name */

  if (jobserver_semaphore == NULL)
    {
      DWORD err = GetLastError ();
      const char *estr = map_windows32_error_to_string (err);
      error (NILF, strlen (auth) + INTSTR_LENGTH + strlen (estr),
             _("unable to open jobserver semaphore '%s': (Error %ld: %s)"),
             auth, err, estr);
      return 0;
    }

  DB (DB_JOBS, (_("Jobserver client (semaphore %s)\n"), auth));

  return 1;
}

char *
jobserver_get_auth ()
{
  return xstrdup (jobserver_semaphore_name);
}

const char *
jobserver_get_invalid_auth ()
{
  /* Because we're using a semaphore we don't need to invalidate.  */
  return NULL;
}

unsigned int
jobserver_enabled ()
{
  return jobserver_semaphore != NULL;
}

/* Close jobserver semaphore */
void
jobserver_clear ()
{
  if (jobserver_semaphore != NULL)
    {
      CloseHandle (jobserver_semaphore);
      jobserver_semaphore = NULL;
    }
}

void
jobserver_release (int is_fatal)
{
  if (! ReleaseSemaphore (
          jobserver_semaphore,    /* handle to semaphore */
          1,                      /* increase count by one */
          NULL))                  /* not interested in previous count */
    {
      if (is_fatal)
        {
          DWORD err = GetLastError ();
          const char *estr = map_windows32_error_to_string (err);
          ONS (fatal, NILF,
               _("release jobserver semaphore: (Error %ld: %s)"), err, estr);
        }
      perror_with_name ("release_jobserver_semaphore", "");
    }
}

unsigned int
jobserver_acquire_all ()
{
  unsigned int tokens = 0;
  while (1)
    {
      DWORD dwEvent = WaitForSingleObject (
          jobserver_semaphore,    /* Handle to semaphore */
          0);                     /* DON'T wait on semaphore */

      if (dwEvent != WAIT_OBJECT_0)
        return tokens;

      ++tokens;
    }
}

void
jobserver_signal ()
{
}

void jobserver_pre_child (int recursive)
{
}

void jobserver_post_child (int recursive)
{
}

void
jobserver_pre_acquire ()
{
}

/* Returns 1 if we got a token, or 0 if a child has completed.
   The Windows implementation doesn't support load detection.  */
unsigned int
jobserver_acquire (int timeout)
{
    HANDLE *handles;
    DWORD dwHandleCount;
    DWORD dwEvent;

    handles = xmalloc(process_table_actual_size() * sizeof(HANDLE));

    /* Add jobserver semaphore to first slot. */
    handles[0] = jobserver_semaphore;

    /* Build array of handles to wait for.  */
    dwHandleCount = 1 + process_set_handles (&handles[1]);

    dwEvent = process_wait_for_multiple_objects (
        dwHandleCount,  /* number of objects in array */
        handles,        /* array of objects */
        FALSE,          /* wait for any object */
        INFINITE);      /* wait until object is signalled */

    free(handles);

    if (dwEvent == WAIT_FAILED)
      {
        DWORD err = GetLastError ();
        const char *estr = map_windows32_error_to_string (err);
        ONS (fatal, NILF,
             _("semaphore or child process wait: (Error %ld: %s)"),
             err, estr);
      }

    /* WAIT_OBJECT_0 indicates that the semaphore was signalled.  */
    return dwEvent == WAIT_OBJECT_0;
}

#endif /* MAKE_JOBSERVER */

#if !defined(NO_OUTPUT_SYNC)

#define MUTEX_PREFIX    "fnm:"

/* Since we're using this with CreateMutex, NULL is invalid.  */
static HANDLE osync_handle = NULL;

unsigned int
osync_enabled ()
{
  return osync_handle != NULL;
}

void
osync_setup ()
{
  SECURITY_ATTRIBUTES secattr;

  /* We are the top-level make, and we want the handle to be inherited
     by our child processes.  */
  secattr.nLength = sizeof (secattr);
  secattr.lpSecurityDescriptor = NULL; /* use default security descriptor */
  secattr.bInheritHandle = TRUE;

  osync_handle = CreateMutex (&secattr, FALSE, NULL);
  if (!osync_handle)
    {
      DWORD err = GetLastError ();
      fprintf (stderr, "CreateMutex: error %lu\n", err);
      errno = ENOLCK;
    }
}

char *
osync_get_mutex ()
{
  char *mutex = NULL;

  if (osync_enabled ())
    {
      /* Prepare the mutex handle string for our children.
         2 hex digits per byte + 2 characters for "0x" + null.  */
      mutex = xmalloc ((2 * sizeof (osync_handle)) + 2 + 1);
      sprintf (mutex, "0x%Ix", (unsigned long long)(DWORD_PTR)osync_handle);
    }

  return mutex;
}

unsigned int
osync_parse_mutex (const char *mutex)
{
  char *endp;
  unsigned long long i;

  errno = 0;
  i = strtoull (mutex, &endp, 16);
  if (errno != 0)
    OSS (fatal, NILF, _("cannot parse output sync mutex %s: %s"),
         mutex, strerror (errno));
  if (endp[0] != '\0')
    OS (fatal, NILF, _("invalid output sync mutex: %s"), mutex);

  osync_handle = (HANDLE) (DWORD_PTR) i;

  return 1;
}

void
osync_clear ()
{
  if (osync_handle)
    {
      CloseHandle (osync_handle);
      osync_handle = NULL;
    }
}

unsigned int
osync_acquire ()
{
  if (osync_enabled())
    {
      DWORD result = WaitForSingleObject (osync_handle, INFINITE);
      if (result == WAIT_FAILED || result == WAIT_TIMEOUT)
        return 0;
    }

  return 1;
}

void
osync_release ()
{
  if (osync_enabled())
    /* FIXME: Perhaps we should call ReleaseMutex repatedly until it errors
       out, to make sure the mutext is released even if we somehow managed to
       to take ownership multiple times?  */
    ReleaseMutex (osync_handle);
}

#endif /* NO_OUTPUT_SYNC */

void
fd_inherit(int fd)
{
  HANDLE fh = (HANDLE)_get_osfhandle(fd);

  if (fh && fh != INVALID_HANDLE_VALUE)
        SetHandleInformation(fh, HANDLE_FLAG_INHERIT, 1);
}

void
fd_noinherit(int fd)
{
  HANDLE fh = (HANDLE)_get_osfhandle(fd);

  if (fh && fh != INVALID_HANDLE_VALUE)
        SetHandleInformation(fh, HANDLE_FLAG_INHERIT, 0);
}

void
fd_set_append (int fd)
{}
