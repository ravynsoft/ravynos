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

#ifndef _STAT_W32_H
#define _STAT_W32_H 1

/* Converts a FILETIME to GMT time since 1970-01-01 00:00:00.  */
#if _GL_WINDOWS_STAT_TIMESPEC
extern struct timespec _gl_convert_FILETIME_to_timespec (const FILETIME *ft);
#else
extern time_t _gl_convert_FILETIME_to_POSIX (const FILETIME *ft);
#endif

/* Fill *BUF with information about the file designated by H.
   PATH is the file name, if known, otherwise NULL.
   Return 0 if successful, or -1 with errno set upon failure.  */
extern int _gl_fstat_by_handle (HANDLE h, const char *path, struct stat *buf);

/* Bitmasks for st_mode.  */
#define S_IREAD_UGO  (_S_IREAD | (_S_IREAD >> 3) | (_S_IREAD >> 6))
#define S_IWRITE_UGO (_S_IWRITE | (_S_IWRITE >> 3) | (_S_IWRITE >> 6))
#define S_IEXEC_UGO  (_S_IEXEC | (_S_IEXEC >> 3) | (_S_IEXEC >> 6))

#endif /* _STAT_W32_H */
