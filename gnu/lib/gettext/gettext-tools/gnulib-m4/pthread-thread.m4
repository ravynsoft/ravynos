# pthread-thread.m4 serial 3
dnl Copyright (C) 2019-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_PTHREAD_THREAD],
[
  AC_REQUIRE([gl_PTHREAD_H])
  AC_REQUIRE([AC_CANONICAL_HOST])

  if { case "$host_os" in mingw* | windows*) true;; *) false;; esac; } \
     && test $gl_threads_api = windows; then
    dnl Choose function names that don't conflict with the mingw-w64 winpthreads
    dnl library.
    REPLACE_PTHREAD_CREATE=1
    REPLACE_PTHREAD_ATTR_INIT=1
    REPLACE_PTHREAD_ATTR_GETDETACHSTATE=1
    REPLACE_PTHREAD_ATTR_SETDETACHSTATE=1
    REPLACE_PTHREAD_ATTR_DESTROY=1
    REPLACE_PTHREAD_SELF=1
    REPLACE_PTHREAD_EQUAL=1
    REPLACE_PTHREAD_DETACH=1
    REPLACE_PTHREAD_JOIN=1
    REPLACE_PTHREAD_EXIT=1
  else
    if test $HAVE_PTHREAD_H = 0; then
      HAVE_PTHREAD_CREATE=0
      HAVE_PTHREAD_ATTR_INIT=0
      HAVE_PTHREAD_ATTR_GETDETACHSTATE=0
      HAVE_PTHREAD_ATTR_SETDETACHSTATE=0
      HAVE_PTHREAD_ATTR_DESTROY=0
      HAVE_PTHREAD_SELF=0
      HAVE_PTHREAD_EQUAL=0
      HAVE_PTHREAD_DETACH=0
      HAVE_PTHREAD_JOIN=0
      HAVE_PTHREAD_EXIT=0
    else
      dnl On HP-UX 11.11, pthread_create() and pthread_attr_init() are only
      dnl defined as inline functions.
      AC_CACHE_CHECK([whether pthread_create exists as a global function],
        [gl_cv_func_pthread_create],
        [saved_LIBS="$LIBS"
         LIBS="$LIBS $LIBPMULTITHREAD"
         AC_LINK_IFELSE(
           [AC_LANG_SOURCE(
              [[extern
                #ifdef __cplusplus
                "C"
                #endif
                int pthread_create (void);
                int main ()
                {
                  return pthread_create ();
                }
              ]])],
           [gl_cv_func_pthread_create=yes],
           [gl_cv_func_pthread_create=no])
         LIBS="$saved_LIBS"
        ])
      if test $gl_cv_func_pthread_create = no; then
        REPLACE_PTHREAD_CREATE=1
        REPLACE_PTHREAD_ATTR_INIT=1
        AC_DEFINE([PTHREAD_CREATE_IS_INLINE], [1],
          [Define if pthread_create is an inline function.])
      fi
    fi
  fi
])
