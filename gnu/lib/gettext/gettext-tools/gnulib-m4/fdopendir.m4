# serial 15
# See if we need to provide fdopendir.

dnl Copyright (C) 2009-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

# Written by Eric Blake.

AC_DEFUN([gl_FUNC_FDOPENDIR],
[
  AC_REQUIRE([gl_DIRENT_H_DEFAULTS])
  AC_REQUIRE([AC_CANONICAL_HOST]) dnl for cross-compiles

  AC_REQUIRE([gl_USE_SYSTEM_EXTENSIONS])

  dnl FreeBSD 7.3 has the function, but failed to declare it.
  AC_CHECK_DECLS([fdopendir], [], [HAVE_DECL_FDOPENDIR=0], [[
#include <dirent.h>
    ]])
  AC_CHECK_FUNCS_ONCE([fdopendir])
  if test $ac_cv_func_fdopendir = no; then
    HAVE_FDOPENDIR=0
  else
    AC_CACHE_CHECK([whether fdopendir works],
      [gl_cv_func_fdopendir_works],
      [AC_RUN_IFELSE(
         [AC_LANG_PROGRAM([[
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
]GL_MDA_DEFINES[
#if !HAVE_DECL_FDOPENDIR
extern
# ifdef __cplusplus
"C"
# endif
DIR *fdopendir (int);
#endif
]],
            [[int result = 0;
              int fd = open ("conftest.c", O_RDONLY);
              if (fd < 0) result |= 1;
              if (fdopendir (fd)) result |= 2;
              if (close (fd)) result |= 4;
              return result;
            ]])],
         [gl_cv_func_fdopendir_works=yes],
         [gl_cv_func_fdopendir_works=no],
         [case "$host_os" in
                                # Guess yes on glibc systems.
            *-gnu*)             gl_cv_func_fdopendir_works="guessing yes" ;;
                                # Guess yes on musl systems.
            *-musl* | midipix*) gl_cv_func_fdopendir_works="guessing yes" ;;
                                # If we don't know, obey --enable-cross-guesses.
            *)                  gl_cv_func_fdopendir_works="$gl_cross_guess_normal" ;;
          esac
         ])])
    case "$gl_cv_func_fdopendir_works" in
      *yes) ;;
      *)
        REPLACE_FDOPENDIR=1
        ;;
    esac
  fi
])
