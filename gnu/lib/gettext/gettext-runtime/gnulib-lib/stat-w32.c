/* Core of implementation of fstat and stat for native Windows.
   Copyright (C) 2017-2023 Free Software Foundation, Inc.

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

/* Written by Bruno Haible.  */

#include <config.h>

#if defined _WIN32 && ! defined __CYGWIN__

/* Attempt to make <windows.h> define FILE_ID_INFO.
   But ensure that the redefinition of _WIN32_WINNT does not make us assume
   Windows Vista or newer when building for an older version of Windows.  */
#if HAVE_SDKDDKVER_H
# include <sdkddkver.h>
# if _WIN32_WINNT >= _WIN32_WINNT_VISTA
#  define WIN32_ASSUME_VISTA 1
# else
#  define WIN32_ASSUME_VISTA 0
# endif
# if !defined _WIN32_WINNT || (_WIN32_WINNT < _WIN32_WINNT_WIN8)
#  undef _WIN32_WINNT
#  define _WIN32_WINNT _WIN32_WINNT_WIN8
# endif
#else
# define WIN32_ASSUME_VISTA (_WIN32_WINNT >= _WIN32_WINNT_VISTA)
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <windows.h>

/* Specification.  */
#include "stat-w32.h"

#include "pathmax.h"

/* Don't assume that UNICODE is not defined.  */
#undef LoadLibrary
#define LoadLibrary LoadLibraryA
#undef GetFinalPathNameByHandle
#define GetFinalPathNameByHandle GetFinalPathNameByHandleA

/* Older mingw headers do not define VOLUME_NAME_NONE.  */
#ifndef VOLUME_NAME_NONE
# define VOLUME_NAME_NONE 4
#endif

#if !WIN32_ASSUME_VISTA

/* Avoid warnings from gcc -Wcast-function-type.  */
# define GetProcAddress \
   (void *) GetProcAddress

# if _GL_WINDOWS_STAT_INODES == 2
/* GetFileInformationByHandleEx was introduced only in Windows Vista.  */
typedef DWORD (WINAPI * GetFileInformationByHandleExFuncType) (HANDLE hFile,
                                                               FILE_INFO_BY_HANDLE_CLASS fiClass,
                                                               LPVOID lpBuffer,
                                                               DWORD dwBufferSize);
static GetFileInformationByHandleExFuncType GetFileInformationByHandleExFunc = NULL;
# endif
/* GetFinalPathNameByHandle was introduced only in Windows Vista.  */
typedef DWORD (WINAPI * GetFinalPathNameByHandleFuncType) (HANDLE hFile,
                                                           LPSTR lpFilePath,
                                                           DWORD lenFilePath,
                                                           DWORD dwFlags);
static GetFinalPathNameByHandleFuncType GetFinalPathNameByHandleFunc = NULL;
static BOOL initialized = FALSE;

static void
initialize (void)
{
  HMODULE kernel32 = LoadLibrary ("kernel32.dll");
  if (kernel32 != NULL)
    {
# if _GL_WINDOWS_STAT_INODES == 2
      GetFileInformationByHandleExFunc =
        (GetFileInformationByHandleExFuncType) GetProcAddress (kernel32, "GetFileInformationByHandleEx");
# endif
      GetFinalPathNameByHandleFunc =
        (GetFinalPathNameByHandleFuncType) GetProcAddress (kernel32, "GetFinalPathNameByHandleA");
    }
  initialized = TRUE;
}

#else

# define GetFileInformationByHandleExFunc GetFileInformationByHandleEx
# define GetFinalPathNameByHandleFunc GetFinalPathNameByHandle

#endif

/* Converts a FILETIME to GMT time since 1970-01-01 00:00:00.  */
#if _GL_WINDOWS_STAT_TIMESPEC
struct timespec
_gl_convert_FILETIME_to_timespec (const FILETIME *ft)
{
  struct timespec result;
  /* FILETIME: <https://docs.microsoft.com/en-us/windows/desktop/api/minwinbase/ns-minwinbase-filetime> */
  unsigned long long since_1601 =
    ((unsigned long long) ft->dwHighDateTime << 32)
    | (unsigned long long) ft->dwLowDateTime;
  if (since_1601 == 0)
    {
      result.tv_sec = 0;
      result.tv_nsec = 0;
    }
  else
    {
      /* Between 1601-01-01 and 1970-01-01 there were 280 normal years and 89
         leap years, in total 134774 days.  */
      unsigned long long since_1970 =
        since_1601 - (unsigned long long) 134774 * (unsigned long long) 86400 * (unsigned long long) 10000000;
      result.tv_sec = since_1970 / (unsigned long long) 10000000;
      result.tv_nsec = (unsigned long) (since_1970 % (unsigned long long) 10000000) * 100;
    }
  return result;
}
#else
time_t
_gl_convert_FILETIME_to_POSIX (const FILETIME *ft)
{
  /* FILETIME: <https://docs.microsoft.com/en-us/windows/desktop/api/minwinbase/ns-minwinbase-filetime> */
  unsigned long long since_1601 =
    ((unsigned long long) ft->dwHighDateTime << 32)
    | (unsigned long long) ft->dwLowDateTime;
  if (since_1601 == 0)
    return 0;
  else
    {
      /* Between 1601-01-01 and 1970-01-01 there were 280 normal years and 89
         leap years, in total 134774 days.  */
      unsigned long long since_1970 =
        since_1601 - (unsigned long long) 134774 * (unsigned long long) 86400 * (unsigned long long) 10000000;
      return since_1970 / (unsigned long long) 10000000;
    }
}
#endif

/* Fill *BUF with information about the file designated by H.
   PATH is the file name, if known, otherwise NULL.
   Return 0 if successful, or -1 with errno set upon failure.  */
int
_gl_fstat_by_handle (HANDLE h, const char *path, struct stat *buf)
{
  /* GetFileType
     <https://docs.microsoft.com/en-us/windows/desktop/api/fileapi/nf-fileapi-getfiletype> */
  DWORD type = GetFileType (h);
  if (type == FILE_TYPE_DISK)
    {
#if !WIN32_ASSUME_VISTA
      if (!initialized)
        initialize ();
#endif

      /* st_mode can be determined through
         GetFileAttributesEx
         <https://docs.microsoft.com/en-us/windows/desktop/api/fileapi/nf-fileapi-getfileattributesexa>
         <https://docs.microsoft.com/en-us/windows/desktop/api/fileapi/ns-fileapi-_win32_file_attribute_data>
         or through
         GetFileInformationByHandle
         <https://docs.microsoft.com/en-us/windows/desktop/api/fileapi/nf-fileapi-getfileinformationbyhandle>
         <https://docs.microsoft.com/en-us/windows/desktop/api/fileapi/ns-fileapi-_by_handle_file_information>
         or through
         GetFileInformationByHandleEx with argument FileBasicInfo
         <https://docs.microsoft.com/en-us/windows/desktop/api/winbase/nf-winbase-getfileinformationbyhandleex>
         <https://docs.microsoft.com/en-us/windows/desktop/api/winbase/ns-winbase-_file_basic_info>
         The latter requires -D_WIN32_WINNT=_WIN32_WINNT_VISTA or higher.  */
      BY_HANDLE_FILE_INFORMATION info;
      if (! GetFileInformationByHandle (h, &info))
        goto failed;

      /* Test for error conditions before starting to fill *buf.  */
      if (sizeof (buf->st_size) <= 4 && info.nFileSizeHigh > 0)
        {
          errno = EOVERFLOW;
          return -1;
        }

#if _GL_WINDOWS_STAT_INODES
      /* st_ino can be determined through
         GetFileInformationByHandle
         <https://docs.microsoft.com/en-us/windows/desktop/api/fileapi/nf-fileapi-getfileinformationbyhandle>
         <https://docs.microsoft.com/en-us/windows/desktop/api/fileapi/ns-fileapi-_by_handle_file_information>
         as 64 bits, or through
         GetFileInformationByHandleEx with argument FileIdInfo
         <https://docs.microsoft.com/en-us/windows/desktop/api/winbase/nf-winbase-getfileinformationbyhandleex>
         <https://docs.microsoft.com/en-us/windows/desktop/api/winbase/ns-winbase-_file_id_info>
         as 128 bits.
         The latter requires -D_WIN32_WINNT=_WIN32_WINNT_WIN8 or higher.  */
      /* Experiments show that GetFileInformationByHandleEx does not provide
         much more information than GetFileInformationByHandle:
           * The dwVolumeSerialNumber from GetFileInformationByHandle is equal
             to the low 32 bits of the 64-bit VolumeSerialNumber from
             GetFileInformationByHandleEx, and is apparently sufficient for
             identifying the device.
           * The nFileIndex from GetFileInformationByHandle is equal to the low
             64 bits of the 128-bit FileId from GetFileInformationByHandleEx,
             and the high 64 bits of this 128-bit FileId are zero.
           * On a FAT file system, GetFileInformationByHandleEx fails with error
             ERROR_INVALID_PARAMETER, whereas GetFileInformationByHandle
             succeeds.
           * On a CIFS/SMB file system, GetFileInformationByHandleEx fails with
             error ERROR_INVALID_LEVEL, whereas GetFileInformationByHandle
             succeeds.  */
# if _GL_WINDOWS_STAT_INODES == 2
      if (GetFileInformationByHandleExFunc != NULL)
        {
          FILE_ID_INFO id;
          if (GetFileInformationByHandleExFunc (h, FileIdInfo, &id, sizeof (id)))
            {
              buf->st_dev = id.VolumeSerialNumber;
              static_assert (sizeof (ino_t) == sizeof (id.FileId));
              memcpy (&buf->st_ino, &id.FileId, sizeof (ino_t));
              goto ino_done;
            }
          else
            {
              switch (GetLastError ())
                {
                case ERROR_INVALID_PARAMETER: /* older Windows version, or FAT */
                case ERROR_INVALID_LEVEL: /* CIFS/SMB file system */
                  goto fallback;
                default:
                  goto failed;
                }
            }
        }
     fallback: ;
      /* Fallback for older Windows versions.  */
      buf->st_dev = info.dwVolumeSerialNumber;
      buf->st_ino._gl_ino[0] = ((ULONGLONG) info.nFileIndexHigh << 32) | (ULONGLONG) info.nFileIndexLow;
      buf->st_ino._gl_ino[1] = 0;
     ino_done: ;
# else /* _GL_WINDOWS_STAT_INODES == 1 */
      buf->st_dev = info.dwVolumeSerialNumber;
      buf->st_ino = ((ULONGLONG) info.nFileIndexHigh << 32) | (ULONGLONG) info.nFileIndexLow;
# endif
#else
      /* st_ino is not wide enough for identifying a file on a device.
         Without st_ino, st_dev is pointless.  */
      buf->st_dev = 0;
      buf->st_ino = 0;
#endif

      /* st_mode.  */
      unsigned int mode =
        /* XXX How to handle FILE_ATTRIBUTE_REPARSE_POINT ?  */
        ((info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? _S_IFDIR | S_IEXEC_UGO : _S_IFREG)
        | S_IREAD_UGO
        | ((info.dwFileAttributes & FILE_ATTRIBUTE_READONLY) ? 0 : S_IWRITE_UGO);
      if (!(info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
          /* Determine whether the file is executable by looking at the file
             name suffix.
             If the file name is already known, use it. Otherwise, for
             non-empty files, it can be determined through
             GetFinalPathNameByHandle
             <https://docs.microsoft.com/en-us/windows/desktop/api/fileapi/nf-fileapi-getfinalpathnamebyhandlea>
             or through
             GetFileInformationByHandleEx with argument FileNameInfo
             <https://docs.microsoft.com/en-us/windows/desktop/api/winbase/nf-winbase-getfileinformationbyhandleex>
             <https://docs.microsoft.com/en-us/windows/desktop/api/winbase/ns-winbase-_file_name_info>
             Both require -D_WIN32_WINNT=_WIN32_WINNT_VISTA or higher.  */
          if (info.nFileSizeHigh > 0 || info.nFileSizeLow > 0)
            {
              char fpath[PATH_MAX];
              if (path != NULL
                  || (GetFinalPathNameByHandleFunc != NULL
                      && GetFinalPathNameByHandleFunc (h, fpath, sizeof (fpath), VOLUME_NAME_NONE)
                         < sizeof (fpath)
                      && (path = fpath, 1)))
                {
                  const char *last_dot = NULL;
                  const char *p;
                  for (p = path; *p != '\0'; p++)
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
              else
                /* Cannot determine file name.  Pretend that it is executable.  */
                mode |= S_IEXEC_UGO;
            }
        }
      buf->st_mode = mode;

      /* st_nlink can be determined through
         GetFileInformationByHandle
         <https://docs.microsoft.com/en-us/windows/desktop/api/fileapi/nf-fileapi-getfileinformationbyhandle>
         <https://docs.microsoft.com/en-us/windows/desktop/api/fileapi/ns-fileapi-_by_handle_file_information>
         or through
         GetFileInformationByHandleEx with argument FileStandardInfo
         <https://docs.microsoft.com/en-us/windows/desktop/api/winbase/nf-winbase-getfileinformationbyhandleex>
         <https://docs.microsoft.com/en-us/windows/desktop/api/winbase/ns-winbase-_file_standard_info>
         The latter requires -D_WIN32_WINNT=_WIN32_WINNT_VISTA or higher.  */
      buf->st_nlink = (info.nNumberOfLinks > SHRT_MAX ? SHRT_MAX : info.nNumberOfLinks);

      /* There's no easy way to map the Windows SID concept to an integer.  */
      buf->st_uid = 0;
      buf->st_gid = 0;

      /* st_rdev is irrelevant for normal files and directories.  */
      buf->st_rdev = 0;

      /* st_size can be determined through
         GetFileSizeEx
         <https://docs.microsoft.com/en-us/windows/desktop/api/fileapi/nf-fileapi-getfilesizeex>
         or through
         GetFileAttributesEx
         <https://docs.microsoft.com/en-us/windows/desktop/api/fileapi/nf-fileapi-getfileattributesexa>
         <https://docs.microsoft.com/en-us/windows/desktop/api/fileapi/ns-fileapi-_win32_file_attribute_data>
         or through
         GetFileInformationByHandle
         <https://docs.microsoft.com/en-us/windows/desktop/api/fileapi/nf-fileapi-getfileinformationbyhandle>
         <https://docs.microsoft.com/en-us/windows/desktop/api/fileapi/ns-fileapi-_by_handle_file_information>
         or through
         GetFileInformationByHandleEx with argument FileStandardInfo
         <https://docs.microsoft.com/en-us/windows/desktop/api/winbase/nf-winbase-getfileinformationbyhandleex>
         <https://docs.microsoft.com/en-us/windows/desktop/api/winbase/ns-winbase-_file_standard_info>
         The latter requires -D_WIN32_WINNT=_WIN32_WINNT_VISTA or higher.  */
      if (sizeof (buf->st_size) <= 4)
        /* Range check already done above.  */
        buf->st_size = info.nFileSizeLow;
      else
        buf->st_size = ((long long) info.nFileSizeHigh << 32) | (long long) info.nFileSizeLow;

      /* st_atime, st_mtime, st_ctime can be determined through
         GetFileTime
         <https://docs.microsoft.com/en-us/windows/desktop/api/fileapi/nf-fileapi-getfiletime>
         or through
         GetFileAttributesEx
         <https://docs.microsoft.com/en-us/windows/desktop/api/fileapi/nf-fileapi-getfileattributesexa>
         <https://docs.microsoft.com/en-us/windows/desktop/api/fileapi/ns-fileapi-_win32_file_attribute_data>
         or through
         GetFileInformationByHandle
         <https://docs.microsoft.com/en-us/windows/desktop/api/fileapi/nf-fileapi-getfileinformationbyhandle>
         <https://docs.microsoft.com/en-us/windows/desktop/api/fileapi/ns-fileapi-_by_handle_file_information>
         or through
         GetFileInformationByHandleEx with argument FileBasicInfo
         <https://docs.microsoft.com/en-us/windows/desktop/api/winbase/nf-winbase-getfileinformationbyhandleex>
         <https://docs.microsoft.com/en-us/windows/desktop/api/winbase/ns-winbase-_file_basic_info>
         The latter requires -D_WIN32_WINNT=_WIN32_WINNT_VISTA or higher.  */
#if _GL_WINDOWS_STAT_TIMESPEC
      buf->st_atim = _gl_convert_FILETIME_to_timespec (&info.ftLastAccessTime);
      buf->st_mtim = _gl_convert_FILETIME_to_timespec (&info.ftLastWriteTime);
      buf->st_ctim = _gl_convert_FILETIME_to_timespec (&info.ftCreationTime);
#else
      buf->st_atime = _gl_convert_FILETIME_to_POSIX (&info.ftLastAccessTime);
      buf->st_mtime = _gl_convert_FILETIME_to_POSIX (&info.ftLastWriteTime);
      buf->st_ctime = _gl_convert_FILETIME_to_POSIX (&info.ftCreationTime);
#endif

      return 0;
    }
  else if (type == FILE_TYPE_CHAR || type == FILE_TYPE_PIPE)
    {
      buf->st_dev = 0;
#if _GL_WINDOWS_STAT_INODES == 2
      buf->st_ino._gl_ino[0] = buf->st_ino._gl_ino[1] = 0;
#else
      buf->st_ino = 0;
#endif
      buf->st_mode = (type == FILE_TYPE_PIPE ? _S_IFIFO : _S_IFCHR);
      buf->st_nlink = 1;
      buf->st_uid = 0;
      buf->st_gid = 0;
      buf->st_rdev = 0;
      if (type == FILE_TYPE_PIPE)
        {
          /* PeekNamedPipe
             <https://msdn.microsoft.com/en-us/library/aa365779.aspx> */
          DWORD bytes_available;
          if (PeekNamedPipe (h, NULL, 0, NULL, &bytes_available, NULL))
            buf->st_size = bytes_available;
          else
            buf->st_size = 0;
        }
      else
        buf->st_size = 0;
#if _GL_WINDOWS_STAT_TIMESPEC
      buf->st_atim.tv_sec = 0; buf->st_atim.tv_nsec = 0;
      buf->st_mtim.tv_sec = 0; buf->st_mtim.tv_nsec = 0;
      buf->st_ctim.tv_sec = 0; buf->st_ctim.tv_nsec = 0;
#else
      buf->st_atime = 0;
      buf->st_mtime = 0;
      buf->st_ctime = 0;
#endif
      return 0;
    }
  else
    {
      errno = ENOENT;
      return -1;
    }

 failed:
  {
    DWORD error = GetLastError ();
    #if 0
    fprintf (stderr, "_gl_fstat_by_handle error 0x%x\n", (unsigned int) error);
    #endif
    switch (error)
      {
      case ERROR_ACCESS_DENIED:
      case ERROR_SHARING_VIOLATION:
        errno = EACCES;
        break;

      case ERROR_OUTOFMEMORY:
        errno = ENOMEM;
        break;

      case ERROR_WRITE_FAULT:
      case ERROR_READ_FAULT:
      case ERROR_GEN_FAILURE:
        errno = EIO;
        break;

      default:
        errno = EINVAL;
        break;
      }
    return -1;
  }
}

#else

/* This declaration is solely to ensure that after preprocessing
   this file is never empty.  */
typedef int dummy;

#endif
