/* Work around platform bugs in utime.
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

/* Written by Bruno Haible.  */

#include <config.h>

/* Specification.  */
#include <utime.h>

#if defined _WIN32 && ! defined __CYGWIN__

# include <errno.h>
# include <windows.h>
# include "filename.h"
# include "malloca.h"

/* Don't assume that UNICODE is not defined.  */
# undef CreateFile
# define CreateFile CreateFileA
# undef GetFileAttributes
# define GetFileAttributes GetFileAttributesA

int
_gl_utimens_windows (const char *name, struct timespec ts[2])
{
  /* POSIX <https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap04.html#tag_04_13>
     specifies: "More than two leading <slash> characters shall be treated as
     a single <slash> character."  */
  if (ISSLASH (name[0]) && ISSLASH (name[1]) && ISSLASH (name[2]))
    {
      name += 2;
      while (ISSLASH (name[1]))
        name++;
    }

  size_t len = strlen (name);
  size_t drive_prefix_len = (HAS_DEVICE (name) ? 2 : 0);

  /* Remove trailing slashes (except the very first one, at position
     drive_prefix_len), but remember their presence.  */
  size_t rlen;
  bool check_dir = false;

  rlen = len;
  while (rlen > drive_prefix_len && ISSLASH (name[rlen-1]))
    {
      check_dir = true;
      if (rlen == drive_prefix_len + 1)
        break;
      rlen--;
    }

  const char *rname;
  char *malloca_rname;
  if (rlen == len)
    {
      rname = name;
      malloca_rname = NULL;
    }
  else
    {
      malloca_rname = malloca (rlen + 1);
      if (malloca_rname == NULL)
        {
          errno = ENOMEM;
          return -1;
        }
      memcpy (malloca_rname, name, rlen);
      malloca_rname[rlen] = '\0';
      rname = malloca_rname;
    }

  DWORD error;

  /* Open a handle to the file.
     CreateFile
     <https://docs.microsoft.com/en-us/windows/desktop/api/fileapi/nf-fileapi-createfilea>
     <https://docs.microsoft.com/en-us/windows/desktop/FileIO/creating-and-opening-files>  */
  HANDLE handle =
    CreateFile (rname,
                FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                NULL,
                OPEN_EXISTING,
                /* FILE_FLAG_POSIX_SEMANTICS (treat file names that differ only
                   in case as different) makes sense only when applied to *all*
                   filesystem operations.  */
                FILE_FLAG_BACKUP_SEMANTICS /* | FILE_FLAG_POSIX_SEMANTICS */,
                NULL);
  if (handle == INVALID_HANDLE_VALUE)
    {
      error = GetLastError ();
      goto failed;
    }

  if (check_dir)
    {
      /* GetFileAttributes
         <https://docs.microsoft.com/en-us/windows/desktop/api/fileapi/nf-fileapi-getfileattributesa>  */
      DWORD attributes = GetFileAttributes (rname);
      if (attributes == INVALID_FILE_ATTRIBUTES)
        {
          error = GetLastError ();
          CloseHandle (handle);
          goto failed;
        }
      if ((attributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
        {
          CloseHandle (handle);
          if (malloca_rname != NULL)
            freea (malloca_rname);
          errno = ENOTDIR;
          return -1;
        }
    }

  {
    /* Use SetFileTime(). See
       <https://docs.microsoft.com/en-us/windows/desktop/api/fileapi/nf-fileapi-setfiletime>
       <https://docs.microsoft.com/en-us/windows/desktop/api/minwinbase/ns-minwinbase-filetime>  */
    FILETIME last_access_time;
    FILETIME last_write_time;
    if (ts == NULL)
      {
        /* GetSystemTimeAsFileTime is the same as
           GetSystemTime followed by SystemTimeToFileTime.
           <https://docs.microsoft.com/en-us/windows/desktop/api/sysinfoapi/nf-sysinfoapi-getsystemtimeasfiletime>.
           It would be overkill to use
           GetSystemTimePreciseAsFileTime
           <https://docs.microsoft.com/en-us/windows/desktop/api/sysinfoapi/nf-sysinfoapi-getsystemtimepreciseasfiletime>.  */
        FILETIME current_time;
        GetSystemTimeAsFileTime (&current_time);
        last_access_time = current_time;
        last_write_time = current_time;
      }
    else
      {
        {
          ULONGLONG time_since_16010101 =
            (ULONGLONG) ts[0].tv_sec * 10000000 + ts[0].tv_nsec / 100 + 116444736000000000LL;
          last_access_time.dwLowDateTime = (DWORD) time_since_16010101;
          last_access_time.dwHighDateTime = time_since_16010101 >> 32;
        }
        {
          ULONGLONG time_since_16010101 =
            (ULONGLONG) ts[1].tv_sec * 10000000 + ts[1].tv_nsec / 100 + 116444736000000000LL;
          last_write_time.dwLowDateTime = (DWORD) time_since_16010101;
          last_write_time.dwHighDateTime = time_since_16010101 >> 32;
        }
      }
    if (SetFileTime (handle, NULL, &last_access_time, &last_write_time))
      {
        CloseHandle (handle);
        if (malloca_rname != NULL)
          freea (malloca_rname);
        return 0;
      }
    else
      {
        #if 0
        DWORD sft_error = GetLastError ();
        fprintf (stderr, "utimens SetFileTime error 0x%x\n", (unsigned int) sft_error);
        #endif
        CloseHandle (handle);
        if (malloca_rname != NULL)
          freea (malloca_rname);
        errno = EINVAL;
        return -1;
      }
  }

 failed:
  {
    #if 0
    fprintf (stderr, "utimens CreateFile/GetFileAttributes error 0x%x\n", (unsigned int) error);
    #endif
    if (malloca_rname != NULL)
      freea (malloca_rname);

    switch (error)
      {
      /* Some of these errors probably cannot happen with the specific flags
         that we pass to CreateFile.  But who knows...  */
      case ERROR_FILE_NOT_FOUND: /* The last component of rname does not exist.  */
      case ERROR_PATH_NOT_FOUND: /* Some directory component in rname does not exist.  */
      case ERROR_BAD_PATHNAME:   /* rname is such as '\\server'.  */
      case ERROR_BAD_NETPATH:    /* rname is such as '\\nonexistentserver\share'.  */
      case ERROR_BAD_NET_NAME:   /* rname is such as '\\server\nonexistentshare'.  */
      case ERROR_INVALID_NAME:   /* rname contains wildcards, misplaced colon, etc.  */
      case ERROR_DIRECTORY:
        errno = ENOENT;
        break;

      case ERROR_ACCESS_DENIED:  /* rname is such as 'C:\System Volume Information\foo'.  */
      case ERROR_SHARING_VIOLATION: /* rname is such as 'C:\pagefile.sys'.  */
        errno = (ts != NULL ? EPERM : EACCES);
        break;

      case ERROR_OUTOFMEMORY:
        errno = ENOMEM;
        break;

      case ERROR_WRITE_PROTECT:
        errno = EROFS;
        break;

      case ERROR_WRITE_FAULT:
      case ERROR_READ_FAULT:
      case ERROR_GEN_FAILURE:
        errno = EIO;
        break;

      case ERROR_BUFFER_OVERFLOW:
      case ERROR_FILENAME_EXCED_RANGE:
        errno = ENAMETOOLONG;
        break;

      case ERROR_DELETE_PENDING: /* XXX map to EACCES or EPERM? */
        errno = EPERM;
        break;

      default:
        errno = EINVAL;
        break;
      }

    return -1;
  }
}

int
utime (const char *name, const struct utimbuf *ts)
{
  if (ts == NULL)
    return _gl_utimens_windows (name, NULL);
  else
    {
      struct timespec ts_with_nanoseconds[2];
      ts_with_nanoseconds[0].tv_sec = ts->actime;
      ts_with_nanoseconds[0].tv_nsec = 0;
      ts_with_nanoseconds[1].tv_sec = ts->modtime;
      ts_with_nanoseconds[1].tv_nsec = 0;
      return _gl_utimens_windows (name, ts_with_nanoseconds);
    }
}

#else

# include <errno.h>
# include <sys/stat.h>
# include "filename.h"

int
utime (const char *name, const struct utimbuf *ts)
#undef utime
{
# if REPLACE_FUNC_UTIME_FILE
  /* macOS 10.13 mistakenly succeeds when given a symbolic link to a
     non-directory with a trailing slash.  */
  size_t len = strlen (name);
  if (len > 0 && ISSLASH (name[len - 1]))
    {
      struct stat buf;

      if (stat (name, &buf) == -1 && errno != EOVERFLOW)
        return -1;
    }
# endif /* REPLACE_FUNC_UTIME_FILE */

  return utime (name, ts);
}

#endif
