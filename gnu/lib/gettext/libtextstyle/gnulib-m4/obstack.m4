# See if we need to provide obstacks.

dnl Copyright 1996-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl Autoconf's AC_FUNC_OBSTACK is marked obsolete since version 2.70.
dnl We provide our own macro here.

AC_DEFUN([gl_FUNC_OBSTACK],
[
  AC_CHECK_HEADERS_ONCE([obstack.h])
  if test $ac_cv_header_obstack_h = yes; then
    HAVE_OBSTACK=1
    AC_CACHE_CHECK([for obstacks that work with any size object],
      [gl_cv_func_obstack],
      [AC_LINK_IFELSE(
         [AC_LANG_PROGRAM(
            [[#include "obstack.h"
              void *obstack_chunk_alloc (size_t n) { return 0; }
              void obstack_chunk_free (void *p) { }
              /* Check that an internal function returns size_t, not int.  */
              size_t _obstack_memory_used (struct obstack *);
             ]],
            [[struct obstack mem;
              obstack_init (&mem);
              obstack_free (&mem, 0);
            ]])],
         [gl_cv_func_obstack=yes],
         [gl_cv_func_obstack=no])
      ])
    if test $gl_cv_func_obstack = yes; then
      REPLACE_OBSTACK=0
    else
      REPLACE_OBSTACK=1
    fi
  else
    HAVE_OBSTACK=0
    REPLACE_OBSTACK=0
  fi
  if test $HAVE_OBSTACK = 0 || test $REPLACE_OBSTACK = 1; then
    GL_GENERATE_OBSTACK_H=true
  else
    GL_GENERATE_OBSTACK_H=false
  fi
  AC_SUBST([REPLACE_OBSTACK])
])
