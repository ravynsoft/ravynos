/* Declarations for error-reporting functions.
   Copyright (C) 1995-1997, 2003, 2006, 2008-2023 Free Software Foundation,
   Inc.
   This file is part of the GNU C Library.

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

#ifndef _@GUARD_PREFIX@_ERROR_H

/* No @PRAGMA_SYSTEM_HEADER@ here, because it would prevent
   -Wimplicit-fallthrough warnings for missing FALLTHROUGH after error(...)
   or error_at_line(...) invocations.  */

/* The include_next requires a split double-inclusion guard.  */
#if @HAVE_ERROR_H@
# @INCLUDE_NEXT@ @NEXT_ERROR_H@
#endif

#ifndef _@GUARD_PREFIX@_ERROR_H
#define _@GUARD_PREFIX@_ERROR_H

/* This file uses _GL_ATTRIBUTE_ALWAYS_INLINE, _GL_ATTRIBUTE_FORMAT,
  _GL_ATTRIBUTE_MAYBE_UNUSED.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif

/* Get 'unreachable'.  */
#include <stddef.h>

/* Get _GL_ATTRIBUTE_SPEC_PRINTF_STANDARD, _GL_ATTRIBUTE_SPEC_PRINTF_SYSTEM.  */
#include <stdio.h>

/* The definitions of _GL_FUNCDECL_RPL etc. are copied here.  */

#if GNULIB_VFPRINTF_POSIX
# define _GL_ATTRIBUTE_SPEC_PRINTF_ERROR _GL_ATTRIBUTE_SPEC_PRINTF_STANDARD
#else
# define _GL_ATTRIBUTE_SPEC_PRINTF_ERROR _GL_ATTRIBUTE_SPEC_PRINTF_SYSTEM
#endif

/* Helper macro for supporting the compiler's control flow analysis better.
   It evaluates its arguments only once.
   Test case: Compile copy-file.c with "gcc -Wimplicit-fallthrough".  */
#if defined __GNUC__ || defined __clang__
/* Use 'unreachable' to tell the compiler when the function call does not
   return.  */
# define __gl_error_call1(function, status, ...) \
    ((function) (status, __VA_ARGS__), \
     (status) != 0 ? unreachable () : (void) 0)
/* If STATUS is a not a constant, the function call may or may not return;
   therefore -Wimplicit-fallthrough will produce a warning.  Use a compound
   statement in order to evaluate STATUS only once.
   If STATUS is a constant, we don't use a compound statement, because that
   would trigger a -Wimplicit-fallthrough warning even when STATUS is != 0,
   when not optimizing.  This causes STATUS to be evaluated twice, but
   that's OK since it does not have side effects.  */
# define __gl_error_call(function, status, ...)                 \
    (__builtin_constant_p (status)                              \
     ? __gl_error_call1 (function, status, __VA_ARGS__)         \
     : __extension__                                            \
       ({                                                       \
         int const __errstatus = status;                        \
         __gl_error_call1 (function, __errstatus, __VA_ARGS__); \
       }))
#else
# define __gl_error_call(function, status, ...) \
    (function) (status, __VA_ARGS__)
#endif

#if GNULIB_REPLACE_ERROR
# undef error_print_progname
# undef error_message_count
# undef error_one_per_line
# define error_print_progname rpl_error_print_progname
# define error_message_count rpl_error_message_count
# define error_one_per_line rpl_error_one_per_line
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Print a message with 'fprintf (stderr, FORMAT, ...)';
   if ERRNUM is nonzero, follow it with ": " and strerror (ERRNUM).
   If STATUS is nonzero, terminate the program with 'exit (STATUS)'.  */
#if @REPLACE_ERROR@
# if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#  undef error
#  define error rpl_error
# endif
_GL_FUNCDECL_RPL (error, void,
                  (int __status, int __errnum, const char *__format, ...)
                  _GL_ATTRIBUTE_FORMAT ((_GL_ATTRIBUTE_SPEC_PRINTF_ERROR, 3, 4)));
_GL_CXXALIAS_RPL (error, void,
                  (int __status, int __errnum, const char *__format, ...));
# ifndef _GL_NO_INLINE_ERROR
#  undef error
#  define error(status, ...) \
     __gl_error_call (rpl_error, status, __VA_ARGS__)
# endif
#else
# if ! @HAVE_ERROR@
_GL_FUNCDECL_SYS (error, void,
                  (int __status, int __errnum, const char *__format, ...)
                  _GL_ATTRIBUTE_FORMAT ((_GL_ATTRIBUTE_SPEC_PRINTF_ERROR, 3, 4)));
# endif
_GL_CXXALIAS_SYS (error, void,
                  (int __status, int __errnum, const char *__format, ...));
# ifndef _GL_NO_INLINE_ERROR
#  ifdef error
/* Only gcc ≥ 4.7 has __builtin_va_arg_pack.  */
#   if _GL_GNUC_PREREQ (4, 7)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wattributes"
_GL_ATTRIBUTE_MAYBE_UNUSED
static void
_GL_ATTRIBUTE_ALWAYS_INLINE
_GL_ATTRIBUTE_FORMAT ((_GL_ATTRIBUTE_SPEC_PRINTF_ERROR, 3, 4))
_gl_inline_error (int __status, int __errnum, const char *__format, ...)
{
  return error (__status, __errnum, __format, __builtin_va_arg_pack ());
}
#    pragma GCC diagnostic pop
#    undef error
#    define error(status, ...) \
       __gl_error_call (_gl_inline_error, status, __VA_ARGS__)
#   endif
#  else
#   define error(status, ...) \
      __gl_error_call (error, status, __VA_ARGS__)
#  endif
# endif
#endif
#if __GLIBC__ >= 2
_GL_CXXALIASWARN (error);
#endif

/* Likewise.  If FILENAME is non-NULL, include FILENAME:LINENO: in the
   message.  */
#if @REPLACE_ERROR_AT_LINE@
# if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#  undef error_at_line
#  define error_at_line rpl_error_at_line
# endif
_GL_FUNCDECL_RPL (error_at_line, void,
                  (int __status, int __errnum, const char *__filename,
                   unsigned int __lineno, const char *__format, ...)
                  _GL_ATTRIBUTE_FORMAT ((_GL_ATTRIBUTE_SPEC_PRINTF_ERROR, 5, 6)));
_GL_CXXALIAS_RPL (error_at_line, void,
                  (int __status, int __errnum, const char *__filename,
                   unsigned int __lineno, const char *__format, ...));
# ifndef _GL_NO_INLINE_ERROR
#  undef error_at_line
#  define error_at_line(status, ...) \
     __gl_error_call (rpl_error_at_line, status, __VA_ARGS__)
# endif
#else
# if ! @HAVE_ERROR_AT_LINE@
_GL_FUNCDECL_SYS (error_at_line, void,
                  (int __status, int __errnum, const char *__filename,
                   unsigned int __lineno, const char *__format, ...)
                  _GL_ATTRIBUTE_FORMAT ((_GL_ATTRIBUTE_SPEC_PRINTF_ERROR, 5, 6)));
# endif
_GL_CXXALIAS_SYS (error_at_line, void,
                  (int __status, int __errnum, const char *__filename,
                   unsigned int __lineno, const char *__format, ...));
# ifndef _GL_NO_INLINE_ERROR
#  ifdef error_at_line
/* Only gcc ≥ 4.7 has __builtin_va_arg_pack.  */
#   if _GL_GNUC_PREREQ (4, 7)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wattributes"
_GL_ATTRIBUTE_MAYBE_UNUSED
static void
_GL_ATTRIBUTE_ALWAYS_INLINE
_GL_ATTRIBUTE_FORMAT ((_GL_ATTRIBUTE_SPEC_PRINTF_ERROR, 5, 6))
_gl_inline_error_at_line (int __status, int __errnum, const char *__filename,
                          unsigned int __lineno, const char *__format, ...)
{
  return error_at_line (__status, __errnum, __filename, __lineno, __format,
                        __builtin_va_arg_pack ());
}
#    pragma GCC diagnostic pop
#    undef error_at_line
#    define error_at_line(status, ...) \
       __gl_error_call (_gl_inline_error_at_line, status, __VA_ARGS__)
#   endif
#  else
#   define error_at_line(status, ...) \
      __gl_error_call (error_at_line, status, __VA_ARGS__)
#  endif
# endif
#endif
_GL_CXXALIASWARN (error_at_line);

/* If NULL, error will flush stdout, then print on stderr the program
   name, a colon and a space.  Otherwise, error will call this
   function without parameters instead.  */
extern DLL_VARIABLE void (*error_print_progname) (void);

/* This variable is incremented each time 'error' is called.  */
extern DLL_VARIABLE unsigned int error_message_count;

/* Sometimes we want to have at most one error per line.  This
   variable controls whether this mode is selected or not.  */
extern DLL_VARIABLE int error_one_per_line;

#ifdef __cplusplus
}
#endif

#endif /* _@GUARD_PREFIX@_ERROR_H */
#endif /* _@GUARD_PREFIX@_ERROR_H */
