/* Work around platform bugs in stat.
   Copyright (C) 2009-2023 Free Software Foundation, Inc.

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

/* Written by Eric Blake and Bruno Haible.  */

/* If the user's config.h happens to include <sys/stat.h>, let it include only
   the system's <sys/stat.h> here, so that orig_stat doesn't recurse to
   rpl_stat.  */
#define __need_system_sys_stat_h
#include <config.h>

/* Get the original definition of stat.  It might be defined as a macro.  */
#include <sys/types.h>
#include <sys/stat.h>
#undef __need_system_sys_stat_h

#if defined _WIN32 && ! defined __CYGWIN__
# define WINDOWS_NATIVE
#endif

#if !defined WINDOWS_NATIVE

static int
orig_stat (const char *filename, struct stat *buf)
{
  return stat (filename, buf);
}

#endif

/* Specification.  */
#ifdef __osf__
/* Write "sys/stat.h" here, not <sys/stat.h>, otherwise OSF/1 5.1 DTK cc
   eliminates this include because of the preliminary #include <sys/stat.h>
   above.  */
# include "sys/stat.h"
#else
# include <sys/stat.h>
#endif

#include "stat-time.h"

#include <errno.h>
#include <limits.h>
#include <string.h>
#include "filename.h"
#include "malloca.h"

#ifdef WINDOWS_NATIVE
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
# include "stat-w32.h"
/* Don't assume that UNICODE is not defined.  */
# undef WIN32_FIND_DATA
# define WIN32_FIND_DATA WIN32_FIND_DATAA
# undef CreateFile
# define CreateFile CreateFileA
# undef FindFirstFile
# define FindFirstFile FindFirstFileA
#endif

#ifdef WINDOWS_NATIVE
/* Return TRUE if the given file name denotes an UNC root.  */
static BOOL
is_unc_root (const char *rname)
{
  /* Test whether it has the syntax '\\server\share'.  */
  if (ISSLASH (rname[0]) && ISSLASH (rname[1]))
    {
      /* It starts with two slashes.  Find the next slash.  */
      const char *p = rname + 2;
      const char *q = p;
      while (*q != '\0' && !ISSLASH (*q))
        q++;
      if (q > p && *q != '\0')
        {
          /* Found the next slash at q.  */
          q++;
          const char *r = q;
          while (*r != '\0' && !ISSLASH (*r))
            r++;
          if (r > q && *r == '\0')
            return TRUE;
        }
    }
  return FALSE;
}
#endif

/* Store information about NAME into ST.  Work around bugs with
   trailing slashes.  Mingw has other bugs (such as st_ino always
   being 0 on success) which this wrapper does not work around.  But
   at least this implementation provides the ability to emulate fchdir
   correctly.  */

int
rpl_stat (char const *name, struct stat *buf)
{
#ifdef WINDOWS_NATIVE
  /* Fill the fields ourselves, because the original stat function returns
     values for st_atime, st_mtime, st_ctime that depend on the current time
     zone.  See
     <https://lists.gnu.org/r/bug-gnulib/2017-04/msg00134.html>  */
  /* XXX Should we convert to wchar_t* and prepend '\\?\', in order to work
     around length limitations
     <https://docs.microsoft.com/en-us/windows/desktop/FileIO/naming-a-file> ?  */

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

  /* Handle '' and 'C:'.  */
  if (!check_dir && rlen == drive_prefix_len)
    {
      errno = ENOENT;
      return -1;
    }

  /* Handle '\\'.  */
  if (rlen == 1 && ISSLASH (name[0]) && len >= 2)
    {
      errno = ENOENT;
      return -1;
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

  /* There are two ways to get at the requested information:
       - by scanning the parent directory and examining the relevant
         directory entry,
       - by opening the file directly.
     The first approach fails for root directories (e.g. 'C:\') and
     UNC root directories (e.g. '\\server\share').
     The second approach fails for some system files (e.g. 'C:\pagefile.sys'
     and 'C:\hiberfil.sys'): ERROR_SHARING_VIOLATION.
     The second approach gives more information (in particular, correct
     st_dev, st_ino, st_nlink fields).
     So we use the second approach and, as a fallback except for root and
     UNC root directories, also the first approach.  */
  {
    int ret;

    {
      /* Approach based on the file.  */

      /* Open a handle to the file.
         CreateFile
         <https://docs.microsoft.com/en-us/windows/desktop/api/fileapi/nf-fileapi-createfilea>
         <https://docs.microsoft.com/en-us/windows/desktop/FileIO/creating-and-opening-files>  */
      HANDLE h =
        CreateFile (rname,
                    FILE_READ_ATTRIBUTES,
                    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                    NULL,
                    OPEN_EXISTING,
                    /* FILE_FLAG_POSIX_SEMANTICS (treat file names that differ only
                       in case as different) makes sense only when applied to *all*
                       filesystem operations.  */
                    FILE_FLAG_BACKUP_SEMANTICS /* | FILE_FLAG_POSIX_SEMANTICS */,
                    NULL);
      if (h != INVALID_HANDLE_VALUE)
        {
          ret = _gl_fstat_by_handle (h, rname, buf);
          CloseHandle (h);
          goto done;
        }
    }

    /* Test for root and UNC root directories.  */
    if ((rlen == drive_prefix_len + 1 && ISSLASH (rname[drive_prefix_len]))
        || is_unc_root (rname))
      goto failed;

    /* Fallback.  */
    {
      /* Approach based on the directory entry.  */

      if (strchr (rname, '?') != NULL || strchr (rname, '*') != NULL)
        {
          /* Other Windows API functions would fail with error
             ERROR_INVALID_NAME.  */
          if (malloca_rname != NULL)
            freea (malloca_rname);
          errno = ENOENT;
          return -1;
        }

      /* Get the details about the directory entry.  This can be done through
         FindFirstFile
         <https://docs.microsoft.com/en-us/windows/desktop/api/fileapi/nf-fileapi-findfirstfilea>
         <https://docs.microsoft.com/en-us/windows/desktop/api/minwinbase/ns-minwinbase-_win32_find_dataa>
         or through
         FindFirstFileEx with argument FindExInfoBasic
         <https://docs.microsoft.com/en-us/windows/desktop/api/fileapi/nf-fileapi-findfirstfileexa>
         <https://docs.microsoft.com/en-us/windows/desktop/api/minwinbase/ne-minwinbase-findex_info_levels>
         <https://docs.microsoft.com/en-us/windows/desktop/api/minwinbase/ns-minwinbase-_win32_find_dataa>  */
      WIN32_FIND_DATA info;
      HANDLE h = FindFirstFile (rname, &info);
      if (h == INVALID_HANDLE_VALUE)
        goto failed;

      /* Test for error conditions before starting to fill *buf.  */
      if (sizeof (buf->st_size) <= 4 && info.nFileSizeHigh > 0)
        {
          FindClose (h);
          if (malloca_rname != NULL)
            freea (malloca_rname);
          errno = EOVERFLOW;
          return -1;
        }

# if _GL_WINDOWS_STAT_INODES
      buf->st_dev = 0;
#  if _GL_WINDOWS_STAT_INODES == 2
      buf->st_ino._gl_ino[0] = buf->st_ino._gl_ino[1] = 0;
#  else /* _GL_WINDOWS_STAT_INODES == 1 */
      buf->st_ino = 0;
#  endif
# else
      /* st_ino is not wide enough for identifying a file on a device.
         Without st_ino, st_dev is pointless.  */
      buf->st_dev = 0;
      buf->st_ino = 0;
# endif

      /* st_mode.  */
      unsigned int mode =
        /* XXX How to handle FILE_ATTRIBUTE_REPARSE_POINT ?  */
        ((info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? _S_IFDIR | S_IEXEC_UGO : _S_IFREG)
        | S_IREAD_UGO
        | ((info.dwFileAttributes & FILE_ATTRIBUTE_READONLY) ? 0 : S_IWRITE_UGO);
      if (!(info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
          /* Determine whether the file is executable by looking at the file
             name suffix.  */
          if (info.nFileSizeHigh > 0 || info.nFileSizeLow > 0)
            {
              const char *last_dot = NULL;
              const char *p;
              for (p = info.cFileName; *p != '\0'; p++)
                if (*p == '.')
                  last_dot = p;
              if (last_dot != NULL)
                {
                  const char *suffix = last_dot + 1;
                  if (_stricmp (suffix, "exe") == 0
                      || _stricmp (suffix, "bat") == 0
                      || _stricmp (suffix, "cmd") == 0
                      || _stricmp (suffix, "com") == 0)
                    mode |= S_IEXEC_UGO;
                }
            }
        }
      buf->st_mode = mode;

      /* st_nlink.  Ignore hard links here.  */
      buf->st_nlink = 1;

      /* There's no easy way to map the Windows SID concept to an integer.  */
      buf->st_uid = 0;
      buf->st_gid = 0;

      /* st_rdev is irrelevant for normal files and directories.  */
      buf->st_rdev = 0;

      /* st_size.  */
      if (sizeof (buf->st_size) <= 4)
        /* Range check already done above.  */
        buf->st_size = info.nFileSizeLow;
      else
        buf->st_size = ((long long) info.nFileSizeHigh << 32) | (long long) info.nFileSizeLow;

      /* st_atime, st_mtime, st_ctime.  */
# if _GL_WINDOWS_STAT_TIMESPEC
      buf->st_atim = _gl_convert_FILETIME_to_timespec (&info.ftLastAccessTime);
      buf->st_mtim = _gl_convert_FILETIME_to_timespec (&info.ftLastWriteTime);
      buf->st_ctim = _gl_convert_FILETIME_to_timespec (&info.ftCreationTime);
# else
      buf->st_atime = _gl_convert_FILETIME_to_POSIX (&info.ftLastAccessTime);
      buf->st_mtime = _gl_convert_FILETIME_to_POSIX (&info.ftLastWriteTime);
      buf->st_ctime = _gl_convert_FILETIME_to_POSIX (&info.ftCreationTime);
# endif

      FindClose (h);

      ret = 0;
    }

   done:
    if (ret >= 0 && check_dir && !S_ISDIR (buf->st_mode))
      {
        errno = ENOTDIR;
        ret = -1;
      }
    if (malloca_rname != NULL)
      {
        int saved_errno = errno;
        freea (malloca_rname);
        errno = saved_errno;
      }
    return ret;
  }

 failed:
  {
    DWORD error = GetLastError ();
    #if 0
    fprintf (stderr, "rpl_stat error 0x%x\n", (unsigned int) error);
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
      case ERROR_BAD_NET_NAME:   /* rname is such as '\\server\nonexistentshare'.  */
      case ERROR_INVALID_NAME:   /* rname contains wildcards, misplaced colon, etc.  */
      case ERROR_DIRECTORY:
        errno = ENOENT;
        break;

      case ERROR_ACCESS_DENIED:  /* rname is such as 'C:\System Volume Information\foo'.  */
      case ERROR_SHARING_VIOLATION: /* rname is such as 'C:\pagefile.sys' (second approach only).  */
                                    /* XXX map to EACCES or EPERM? */
        errno = EACCES;
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
#else
  int result = orig_stat (name, buf);
  if (result == 0)
    {
# if REPLACE_FUNC_STAT_FILE
      /* Solaris 9 mistakenly succeeds when given a non-directory with a
         trailing slash.  */
      if (!S_ISDIR (buf->st_mode))
        {
          size_t len = strlen (name);
          if (ISSLASH (name[len - 1]))
            {
              errno = ENOTDIR;
              return -1;
            }
        }
# endif /* REPLACE_FUNC_STAT_FILE */
      result = stat_time_normalize (result, buf);
    }
  return result;
#endif
}
