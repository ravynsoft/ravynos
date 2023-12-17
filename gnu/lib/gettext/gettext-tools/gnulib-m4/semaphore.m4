# semaphore.m4 serial 1
dnl Copyright (C) 2019-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

# Sets LIB_SEMAPHORE to the library needed, in addition to $(LIBMULTITHREAD),
# for getting the <semaphore.h> functions.

AC_DEFUN([gl_SEMAPHORE],
[
  AC_REQUIRE([gl_THREADLIB])
  dnl sem_post is
  dnl   - in libc on macOS, FreeBSD, AIX, IRIX, Solaris 11, Haiku, Cygwin,
  dnl   - in libpthread on glibc systems, OpenBSD,
  dnl   - in libpthread or librt on NetBSD,
  dnl   - in librt on HP-UX 11, OSF/1, Solaris 10.
  dnl On the platforms where -lpthread is needed, it is contained in
  dnl $LIBMULTITHREAD. Therefore, the only library we need to test for is -lrt.
  AC_CACHE_CHECK([for library needed for semaphore functions],
    [gl_cv_semaphore_lib],
    [save_LIBS="$LIBS"
     LIBS="$LIBS $LIBMULTITHREAD"
     AC_LINK_IFELSE(
       [AC_LANG_PROGRAM(
          [[#include <semaphore.h>]],
          [[sem_post ((sem_t *)0);]])],
       [gl_cv_semaphore_lib=none],
       [LIBS="$LIBS -lrt"
        AC_LINK_IFELSE(
          [AC_LANG_PROGRAM(
             [[#include <semaphore.h>]],
             [[sem_post ((sem_t *)0);]])],
          [gl_cv_semaphore_lib='-lrt'],
          [gl_cv_semaphore_lib=none])
       ])
     LIBS="$save_LIBS"
    ])
  if test "x$gl_cv_semaphore_lib" = xnone; then
    LIB_SEMAPHORE=
  else
    LIB_SEMAPHORE="$gl_cv_semaphore_lib"
  fi
  AC_SUBST([LIB_SEMAPHORE])
])
