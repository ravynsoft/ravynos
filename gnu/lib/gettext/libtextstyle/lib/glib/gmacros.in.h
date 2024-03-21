/* GLIB - Library of useful routines for C programming
 * Copyright (C) 2006-2019 Free Software Foundation, Inc.
 *
 * This file is not part of the GNU gettext program, but is used with
 * GNU gettext.
 *
 * The original copyright notice is as follows:
 */

/* GLIB - Library of useful routines for C programming
 * Copyright (C) 1995-1997  Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * Modified by the GLib Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GLib Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GLib at ftp://ftp.gtk.org/pub/gtk/. 
 */

/*
 * Modified by Bruno Haible for use as a gnulib module.
 */

/* This file must not include any other glib header file and must thus
 * not refer to variables from glibconfig.h 
 */

#ifndef __G_MACROS_H__
#define __G_MACROS_H__

/* We include stddef.h to get the system's definition of NULL
 */
#include <stddef.h>

/* Get LONG_MAX.
 */
#include <limits.h>

#if 0

/* Here we provide G_GNUC_EXTENSION as an alias for __extension__,
 * where this is valid. This allows for warningless compilation of
 * "long long" types even in the presence of '-ansi -pedantic'. 
 */
#if     __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 8)
#  define G_GNUC_EXTENSION __extension__
#else
#  define G_GNUC_EXTENSION
#endif

/* Provide macros to feature the GCC function attribute.
 */
#if    __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 96)
#define G_GNUC_PURE                            \
  __attribute__((__pure__))
#define G_GNUC_MALLOC    			\
  __attribute__((__malloc__))
#else
#define G_GNUC_PURE
#define G_GNUC_MALLOC
#endif

#if     __GNUC__ >= 4
#define G_GNUC_NULL_TERMINATED __attribute__((__sentinel__))
#else
#define G_GNUC_NULL_TERMINATED
#endif

#if     __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ > 4)
#define G_GNUC_PRINTF( format_idx, arg_idx )    \
  __attribute__((__format__ (__printf__, format_idx, arg_idx)))
#define G_GNUC_SCANF( format_idx, arg_idx )     \
  __attribute__((__format__ (__scanf__, format_idx, arg_idx)))
#define G_GNUC_FORMAT( arg_idx )                \
  __attribute__((__format_arg__ (arg_idx)))
#define G_GNUC_NORETURN                         \
  __attribute__((__noreturn__))
#define G_GNUC_CONST                            \
  __attribute__((__const__))
#define G_GNUC_UNUSED                           \
  __attribute__((__unused__))
#define G_GNUC_NO_INSTRUMENT			\
  __attribute__((__no_instrument_function__))
#else   /* !__GNUC__ */
#define G_GNUC_PRINTF( format_idx, arg_idx )
#define G_GNUC_SCANF( format_idx, arg_idx )
#define G_GNUC_FORMAT( arg_idx )
#define G_GNUC_NORETURN
#define G_GNUC_CONST
#define G_GNUC_UNUSED
#define G_GNUC_NO_INSTRUMENT
#endif  /* !__GNUC__ */

#if    __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1)
#define G_GNUC_DEPRECATED                            \
  __attribute__((__deprecated__))
#else
#define G_GNUC_DEPRECATED
#endif /* __GNUC__ */

#if    __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4)
#define G_GNUC_WARN_UNUSED_RESULT 		\
  __attribute__((warn_unused_result))
#else
#define G_GNUC_WARN_UNUSED_RESULT
#endif /* __GNUC__ */

/* Wrap the gcc __PRETTY_FUNCTION__ and __FUNCTION__ variables with
 * macros, so we can refer to them as strings unconditionally.
 * usage not-recommended since gcc-3.0
 */
#if defined (__GNUC__) && (__GNUC__ < 3)
#define G_GNUC_FUNCTION         __FUNCTION__
#define G_GNUC_PRETTY_FUNCTION  __PRETTY_FUNCTION__
#else   /* !__GNUC__ */
#define G_GNUC_FUNCTION         ""
#define G_GNUC_PRETTY_FUNCTION  ""
#endif  /* !__GNUC__ */

#define G_STRINGIFY(macro_or_string)	G_STRINGIFY_ARG (macro_or_string)
#define	G_STRINGIFY_ARG(contents)	#contents

/* Provide a string identifying the current code position */
#if defined(__GNUC__) && (__GNUC__ < 3) && !defined(__cplusplus)
#  define G_STRLOC	__FILE__ ":" G_STRINGIFY (__LINE__) ":" __PRETTY_FUNCTION__ "()"
#else
#  define G_STRLOC	__FILE__ ":" G_STRINGIFY (__LINE__)
#endif

/* Provide a string identifying the current function, non-concatenatable */
#if defined (__GNUC__)
#  define G_STRFUNC     ((const char*) (__PRETTY_FUNCTION__))
#elif defined (G_HAVE_ISO_VARARGS)
#  define G_STRFUNC     ((const char*) (__func__))
#else
#  define G_STRFUNC     ((const char*) ("???"))
#endif

/* Guard C code in headers, while including them from C++ */
#ifdef  __cplusplus
# define G_BEGIN_DECLS  extern "C" {
# define G_END_DECLS    }
#else
# define G_BEGIN_DECLS
# define G_END_DECLS
#endif

/* Provide definitions for some commonly used macros.
 *  Some of them are only provided if they haven't already
 *  been defined. It is assumed that if they are already
 *  defined then the current definition is correct.
 */
#ifndef NULL
#  ifdef __cplusplus
#    define NULL        (0L)
#  else /* !__cplusplus */
#    define NULL        ((void*) 0)
#  endif /* !__cplusplus */
#endif

#ifndef	FALSE
#define	FALSE	(0)
#endif

#ifndef	TRUE
#define	TRUE	(!FALSE)
#endif

#undef	MAX
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))

#undef	MIN
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))

#undef	ABS
#define ABS(a)	   (((a) < 0) ? -(a) : (a))

#undef	CLAMP
#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

#endif

/* Count the number of elements in an array. The array must be defined
 * as such; using this with a dynamically allocated array will give
 * incorrect results.
 */
#define G_N_ELEMENTS(arr)		(sizeof (arr) / sizeof ((arr)[0]))

#if 0

/* Macros by analogy to GINT_TO_POINTER, GPOINTER_TO_INT
 */
#define GPOINTER_TO_SIZE(p)	((gsize) (p))
#define GSIZE_TO_POINTER(s)	((gpointer) (gsize) (s))

/* Provide convenience macros for handling structure
 * fields through their offsets.
 */
#define G_STRUCT_OFFSET(struct_type, member)	\
    ((glong) ((guint8*) &((struct_type*) 0)->member))
#define G_STRUCT_MEMBER_P(struct_p, struct_offset)   \
    ((gpointer) ((guint8*) (struct_p) + (glong) (struct_offset)))
#define G_STRUCT_MEMBER(member_type, struct_p, struct_offset)   \
    (*(member_type*) G_STRUCT_MEMBER_P ((struct_p), (struct_offset)))

/* Provide simple macro statement wrappers (adapted from Perl):
 *  G_STMT_START { statements; } G_STMT_END;
 *  can be used as a single statement, as in
 *  if (x) G_STMT_START { ... } G_STMT_END; else ...
 *
 *  When GCC is compiling C code in non-ANSI mode, it will use the
 *  compiler __extension__ to wrap the statements wihin `({' and '})' braces.
 *  When compiling on platforms where configure has defined
 *  HAVE_DOWHILE_MACROS, statements will be wrapped with `do' and `while (0)'.
 *  For any other platforms (SunOS4 is known to have this issue), wrap the
 *  statements with `if (1)' and `else (void) 0'.
 */
#if !(defined (G_STMT_START) && defined (G_STMT_END))
# if defined (__GNUC__) && !defined (__STRICT_ANSI__) && !defined (__cplusplus)
#  define G_STMT_START (void) __extension__ (
#  define G_STMT_END )
# else /* !(__GNUC__ && !__STRICT_ANSI__ && !__cplusplus) */
#  if defined (HAVE_DOWHILE_MACROS)
#   define G_STMT_START do
#   define G_STMT_END while (0)
#  else /* !HAVE_DOWHILE_MACROS */
#   define G_STMT_START if (1)
#   define G_STMT_END else (void) 0
#  endif /* !HAVE_DOWHILE_MACROS */
# endif /* !(__GNUC__ && !__STRICT_ANSI__ && !__cplusplus) */
#endif

/* Allow the app programmer to select whether or not return values
 * (usually char*) are const or not.  Don't try using this feature for
 * functions with C++ linkage.
 */
#ifdef G_DISABLE_CONST_RETURNS
#define G_CONST_RETURN
#else
#define G_CONST_RETURN const
#endif

/*
 * The G_LIKELY and G_UNLIKELY macros let the programmer give hints to 
 * the compiler about the expected result of an expression. Some compilers
 * can use this information for optimizations.
 *
 * The _G_BOOLEAN_EXPR macro is intended to trigger a gcc warning when
 * putting assignments in g_return_if_fail ().  
 */
#if defined(__GNUC__) && (__GNUC__ > 2) && defined(__OPTIMIZE__)
#define _G_BOOLEAN_EXPR(expr)                   \
 __extension__ ({                               \
   int _g_boolean_var_;                         \
   if (expr)                                    \
      _g_boolean_var_ = 1;                      \
   else                                         \
      _g_boolean_var_ = 0;                      \
   _g_boolean_var_;                             \
})
#define G_LIKELY(expr) (__builtin_expect (_G_BOOLEAN_EXPR(expr), 1))
#define G_UNLIKELY(expr) (__builtin_expect (_G_BOOLEAN_EXPR(expr), 0))
#else
#define G_LIKELY(expr) (expr)
#define G_UNLIKELY(expr) (expr)
#endif

#endif

/* Backport from glib-2.18.4.
 */
#define G_MAXLONG LONG_MAX

#endif /* __G_MACROS_H__ */
