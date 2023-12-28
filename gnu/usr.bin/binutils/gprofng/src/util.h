/* Copyright (C) 2021-2023 Free Software Foundation, Inc.
   Contributed by Oracle.

   This file is part of GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#ifndef _PERFAN_UTIL_H
#define _PERFAN_UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdint.h>

#include "gp-defs.h"
#include "gp-time.h"
#include "i18n.h"
#include "debug.h"

#define SWAP_ENDIAN(x)  swapByteOrder((void *) (&(x)), sizeof(x))
#define AppendString(len, arr, ...) len += snprintf(arr + len, sizeof(arr) - len, __VA_ARGS__)
#define ARR_SIZE(x)     (sizeof (x) / sizeof (*(x)))

// Utility routines.

//
// Inline functions
//
// max(a, b) - Return the maximum of two values
inline int
max (int a, int b)
{
  return (a >= b) ? a : b;
}

// min(a, b) - Return the minimum of two values
inline int
min (int a, int b)
{
  return (a <= b) ? a : b;
}

// streq(s1, s2) - Returns 1 if strings are the same, 0 otherwise
inline int
streq (const char *s1, const char *s2)
{
  return strcmp (s1, s2) == 0;
}

// StrChr(str, ch) - Rerurn 'str' if 'ch' does not occur in 'str' or
// a pointer to the next symbol after the first occurrence of 'ch' in 'str'
inline char *
StrChr (char *str, char ch)
{
  char *s = strchr (str, ch);
  return s ? (s + 1) : str;
}

// StrRchr(str, ch) - Rerurn 'str' if 'ch' does not occur in 'str' or
// a pointer to the next symbol after the last occurrence of 'ch' in 'str'
inline char *
StrRchr (char *str, char ch)
{
  char *s = strrchr (str, ch);
  return s ? (s + 1) : str;
}

inline char*
STR (const char *s)
{
  return s ? (char*) s : (char*) NTXT ("NULL");
}

inline char*
get_str (const char *s, const char *s1)
{
  return s ? (char*) s : (char*) s1;
}

inline char *
get_basename (const char* name)
{
  return StrRchr ((char*) name, '/');
}

inline char *
dbe_strdup (const char *str)
{
  return str ? strdup (str) : NULL;
}

inline long
dbe_sstrlen (const char *str)
{
  return str ? (long) strlen (str) : 0;
}

inline int
dbe_strcmp (const char *s1, const char *s2)
{
  return s1 ? (s2 ? strcmp (s1, s2) : 1) : (s2 ? -1 : 0);
}

// tstodouble(t) - Return timestruc_t in (double) seconds
inline double
tstodouble (timestruc_t t)
{
  return (double) t.tv_sec + (double) (t.tv_nsec / 1000000000.0);
}

inline void
hr2timestruc (timestruc_t *d, hrtime_t s)
{
  d->tv_sec = (long) (s / NANOSEC);
  d->tv_nsec = (long) (s % NANOSEC);
}

inline hrtime_t
timestruc2hr (timestruc_t *s)
{
  return (hrtime_t) s->tv_sec * NANOSEC + (hrtime_t) s->tv_nsec;
}

struct stat64;

#if defined(__cplusplus)
extern "C"
{
#endif
  //
  // Declaration of utility functions
  //
  void tsadd (timestruc_t *result, timestruc_t *time);
  void tssub (timestruc_t *result, timestruc_t *time1, timestruc_t *time2);
  int tscmp (timestruc_t *time1, timestruc_t *time2);
  void int_max (int *maximum, int count);
  char *strstr_r (char *s1, const char *s2);
  char *strrpbrk (const char *string, const char *brkset);
  char *read_line (FILE *);
  char *parse_qstring (char *in_str, char **endptr);
  char *parse_fname (char *in_str, char **fcontext);
  int get_paren (const char *name);

  uint64_t crc64 (const char *str, size_t len);
  char *canonical_path (char *path);
  char *get_relative_path (char *name);
  char *get_relative_link (const char *path_to, const char *path_from);
  char *get_prog_name (int basename);
  char *dbe_strndup (const char *str, size_t len);
  int dbe_stat (const char *path, struct stat64 *sbuf);
  int dbe_stat_file (const char *path, struct stat64 *sbuf);
  char *dbe_read_dir (const char *path, const char *format);
  char *dbe_get_processes (const char *format);
  char *dbe_create_directories (const char *pathname);
  char *dbe_delete_file (const char *pathname);
  char *dbe_xml2str (const char *s);
  void swapByteOrder (void *p, size_t sz);
  char *dbe_sprintf (const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
  ssize_t dbe_write (int f, const char *fmt, ...) __attribute__ ((format (printf, 2, 3)));
  char *dbe_create_symlink_to_path (const char *path, const char *dir);
  int64_t read_from_file (int fd, void *buffer, int64_t nbyte);
  uint32_t get_cksum (const char * pathname, char ** errmsg);

#ifdef  __cplusplus
}
int catch_out_of_memory (int (*real_main)(int, char*[]), int argc, char *argv[]);
#endif


#endif /* _UTIL_H */
