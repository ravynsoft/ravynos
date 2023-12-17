/* Decomposed printf argument list.
   Copyright (C) 1999, 2002-2003, 2006-2007, 2011-2023 Free Software
   Foundation, Inc.

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

#ifndef _PRINTF_ARGS_H
#define _PRINTF_ARGS_H

/* This file can be parametrized with the following macros:
     ENABLE_UNISTDIO    Set to 1 to enable the unistdio extensions.
     PRINTF_FETCHARGS   Name of the function to be declared.
     STATIC             Set to 'static' to declare the function static.  */

/* Default parameters.  */
#ifndef PRINTF_FETCHARGS
# define PRINTF_FETCHARGS printf_fetchargs
#endif

/* Get size_t.  */
#include <stddef.h>

/* Get wchar_t.  */
#if HAVE_WCHAR_T
# include <stddef.h>
#endif

/* Get wint_t.  */
#if HAVE_WINT_T
# include <wchar.h>
#endif

/* Get intN_t, uintN_t, intN_fast_t, uintN_fast_t.  */
#include <stdint.h>

/* Get va_list.  */
#include <stdarg.h>


/* Argument types */
typedef enum
{
  TYPE_NONE,
  TYPE_SCHAR,
  TYPE_UCHAR,
  TYPE_SHORT,
  TYPE_USHORT,
  TYPE_INT,
  TYPE_UINT,
  TYPE_LONGINT,
  TYPE_ULONGINT,
  TYPE_LONGLONGINT,
  TYPE_ULONGLONGINT,
  /* According to ISO C 23 § 7.23.6.1, "all exact-width integer types",
     "all minimum-width integer types", and "all fastest minimum-width integer
     types" defined in <stdint.h> should be supported.  But for portability
     between platforms, we support only those with N = 8, 16, 32, 64.  */
  TYPE_INT8_T,
  TYPE_UINT8_T,
  TYPE_INT16_T,
  TYPE_UINT16_T,
  TYPE_INT32_T,
  TYPE_UINT32_T,
  TYPE_INT64_T,
  TYPE_UINT64_T,
  TYPE_INT_FAST8_T,
  TYPE_UINT_FAST8_T,
  TYPE_INT_FAST16_T,
  TYPE_UINT_FAST16_T,
  TYPE_INT_FAST32_T,
  TYPE_UINT_FAST32_T,
  TYPE_INT_FAST64_T,
  TYPE_UINT_FAST64_T,
  TYPE_DOUBLE,
  TYPE_LONGDOUBLE,
  TYPE_CHAR,
#if HAVE_WINT_T
  TYPE_WIDE_CHAR,
#endif
  TYPE_STRING,
#if HAVE_WCHAR_T
  TYPE_WIDE_STRING,
#endif
  TYPE_POINTER,
  TYPE_COUNT_SCHAR_POINTER,
  TYPE_COUNT_SHORT_POINTER,
  TYPE_COUNT_INT_POINTER,
  TYPE_COUNT_LONGINT_POINTER,
  TYPE_COUNT_LONGLONGINT_POINTER,
  TYPE_COUNT_INT8_T_POINTER,
  TYPE_COUNT_INT16_T_POINTER,
  TYPE_COUNT_INT32_T_POINTER,
  TYPE_COUNT_INT64_T_POINTER,
  TYPE_COUNT_INT_FAST8_T_POINTER,
  TYPE_COUNT_INT_FAST16_T_POINTER,
  TYPE_COUNT_INT_FAST32_T_POINTER,
  TYPE_COUNT_INT_FAST64_T_POINTER
#if ENABLE_UNISTDIO
  /* The unistdio extensions.  */
, TYPE_U8_STRING
, TYPE_U16_STRING
, TYPE_U32_STRING
#endif
} arg_type;

/* Polymorphic argument */
typedef struct
{
  arg_type type;
  union
  {
    signed char                 a_schar;
    unsigned char               a_uchar;
    short                       a_short;
    unsigned short              a_ushort;
    int                         a_int;
    unsigned int                a_uint;
    long int                    a_longint;
    unsigned long int           a_ulongint;
    long long int               a_longlongint;
    unsigned long long int      a_ulonglongint;
    int8_t                      a_int8_t;
    uint8_t                     a_uint8_t;
    int16_t                     a_int16_t;
    uint16_t                    a_uint16_t;
    int32_t                     a_int32_t;
    uint32_t                    a_uint32_t;
    int64_t                     a_int64_t;
    uint64_t                    a_uint64_t;
    int_fast8_t                 a_int_fast8_t;
    uint_fast8_t                a_uint_fast8_t;
    int_fast16_t                a_int_fast16_t;
    uint_fast16_t               a_uint_fast16_t;
    int_fast32_t                a_int_fast32_t;
    uint_fast32_t               a_uint_fast32_t;
    int_fast64_t                a_int_fast64_t;
    uint_fast64_t               a_uint_fast64_t;
    float                       a_float;                     /* unused */
    double                      a_double;
    long double                 a_longdouble;
    int                         a_char;
#if HAVE_WINT_T
    wint_t                      a_wide_char;
#endif
    const char*                 a_string;
#if HAVE_WCHAR_T
    const wchar_t*              a_wide_string;
#endif
    void*                       a_pointer;
    signed char *               a_count_schar_pointer;
    short *                     a_count_short_pointer;
    int *                       a_count_int_pointer;
    long int *                  a_count_longint_pointer;
    long long int *             a_count_longlongint_pointer;
    int8_t *                    a_count_int8_t_pointer;
    int16_t *                   a_count_int16_t_pointer;
    int32_t *                   a_count_int32_t_pointer;
    int64_t *                   a_count_int64_t_pointer;
    int_fast8_t *               a_count_int_fast8_t_pointer;
    int_fast16_t *              a_count_int_fast16_t_pointer;
    int_fast32_t *              a_count_int_fast32_t_pointer;
    int_fast64_t *              a_count_int_fast64_t_pointer;
#if ENABLE_UNISTDIO
    /* The unistdio extensions.  */
    const uint8_t *             a_u8_string;
    const uint16_t *            a_u16_string;
    const uint32_t *            a_u32_string;
#endif
  }
  a;
}
argument;

/* Number of directly allocated arguments (no malloc() needed).  */
#define N_DIRECT_ALLOC_ARGUMENTS 7

typedef struct
{
  size_t count;
  argument *arg;
  argument direct_alloc_arg[N_DIRECT_ALLOC_ARGUMENTS];
}
arguments;


/* Fetch the arguments, putting them into a. */
#ifdef STATIC
STATIC
#else
extern
#endif
int PRINTF_FETCHARGS (va_list args, arguments *a);

#endif /* _PRINTF_ARGS_H */
