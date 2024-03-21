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

/* ====================== Substitute for glibconfig.h ====================== */

#include <stddef.h>
#include <sys/types.h>
#include <stdint.h>

typedef uint16_t guint16;
typedef uint32_t guint32;

typedef size_t gsize;
typedef ssize_t gssize;

#define GPOINTER_TO_INT(p)	((gint)  (intptr_t)  (p))
#define GPOINTER_TO_UINT(p)	((guint) (uintptr_t) (p))

#define GINT_TO_POINTER(i)	((gpointer) (intptr_t)  (i))
#define GUINT_TO_POINTER(u)	((gpointer) (uintptr_t) (u))

#define g_memmove memmove

/* ================ Abridged version of for <glib/macros.h> ================ */

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

#if    __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4)
#define G_GNUC_WARN_UNUSED_RESULT 		\
  __attribute__((warn_unused_result))
#else
#define G_GNUC_WARN_UNUSED_RESULT
#endif /* __GNUC__ */

#ifdef  __cplusplus
# define G_BEGIN_DECLS  extern "C" {
# define G_END_DECLS    }
#else
# define G_BEGIN_DECLS
# define G_END_DECLS
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

#define G_STMT_START
#define G_STMT_END

/* ====================== Substitute for <glib/gmem.h> ====================== */

#include "xalloc.h"

#define g_malloc(n) xmalloc (n)
#define g_malloc0(n) xzalloc (n)
#define g_realloc(p,n) xrealloc (p, n)
#define g_free(p) free (p)
#define g_try_malloc(n) xmalloc (n)
#define g_try_malloc0(n) xzalloc (n)
#define g_try_realloc(p,n) xrealloc (p, n)

#define g_new(t,n) ((t *) xnmalloc (n, sizeof (t)))
#define g_new0(t,n) ((t *) xcalloc (n, sizeof (t)))
#define g_try_new(t,n) ((t *) xnmalloc (n, sizeof (t)))
#define g_try_new0(t,n) ((t *) xcalloc (n, sizeof (t)))

/* =================== Substitute for <glib/gmessages.h> =================== */

#include <stdlib.h>

#define g_assert(expr)                 if (!(expr)) abort ()
#define g_assert_not_reached()         abort ()

#define g_return_if_fail(expr)         if (!(expr)) return
#define g_return_val_if_fail(expr,val) if (!(expr)) return (val)
#define g_return_if_reached()          return
#define g_return_val_if_reached(val)   return (val)

#define G_LOG_LEVEL_CRITICAL 0
#define G_LOG_LEVEL_INFO     0
#define G_LOG_LEVEL_DEBUG    0

extern void g_printerr (const char *format, ...) G_GNUC_PRINTF (1, 2);
extern void g_warning (const char *format, ...) G_GNUC_PRINTF (1, 2);
extern void g_log (const char *domain, int level, const char *format, ...) G_GNUC_PRINTF (3, 4);

/* ==================== Substitute for <glib/gprintf.h> ==================== */

#include <stdio.h>

#define g_printf printf
#define g_fprintf fprintf
#define g_sprintf sprintf
#define g_vprintf vprintf
#define g_vfprintf vfprintf
#define g_vsprintf vsprintf
#define g_vasprintf vasprintf

/* ===================== Substitute for <glib/gslice.h> ===================== */

#define g_slice_new(t) XMALLOC (t)
#define g_slice_new0(t) XZALLOC (t)

#define g_slice_free(t,p) free (p)

/* ======================= Helper for <glib/gtypes.h> ======================= */

/* We don't need to export variables from a shared library.  */
#define GLIB_VAR extern

/* ==================== Substitute for <glib/gunicode.h> ==================== */

typedef unsigned int gunichar;
