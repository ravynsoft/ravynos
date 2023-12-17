# getdelim.m4 serial 19

dnl Copyright (C) 2005-2007, 2009-2023 Free Software Foundation, Inc.
dnl
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_PREREQ([2.59])

AC_DEFUN([gl_FUNC_GETDELIM],
[
  AC_REQUIRE([gl_STDIO_H_DEFAULTS])
  AC_REQUIRE([AC_CANONICAL_HOST])

  dnl Persuade glibc <stdio.h> to declare getdelim().
  AC_REQUIRE([AC_USE_SYSTEM_EXTENSIONS])

  AC_CHECK_DECLS_ONCE([getdelim])

  gl_CHECK_FUNCS_ANDROID([getdelim], [[#include <stdio.h>]])
  if test $ac_cv_func_getdelim = yes; then
    HAVE_GETDELIM=1
    dnl Found it in some library.  Verify that it works.
    AC_CACHE_CHECK([for working getdelim function],
      [gl_cv_func_working_getdelim],
      [case "$host_os" in
         darwin*)
           dnl On macOS 10.13, valgrind detected an out-of-bounds read during
           dnl the GNU sed test suite:
           dnl   Invalid read of size 16
           dnl      at 0x100EE6A05: _platform_memchr$VARIANT$Base (in /usr/lib/system/libsystem_platform.dylib)
           dnl      by 0x100B7B0BD: getdelim (in /usr/lib/system/libsystem_c.dylib)
           dnl      by 0x10000B0BE: ck_getdelim (utils.c:254)
           gl_cv_func_working_getdelim=no ;;
         *)
           echo fooNbarN | tr -d '\012' | tr N '\012' > conftest.data
           AC_RUN_IFELSE([AC_LANG_SOURCE([[
#    include <stdio.h>
#    include <stdlib.h>
#    include <string.h>
    int main ()
    {
      FILE *in = fopen ("./conftest.data", "r");
      if (!in)
        return 1;
      {
        /* Test result for a NULL buffer and a zero size.
           Based on a test program from Karl Heuer.  */
        char *line = NULL;
        size_t siz = 0;
        int len = getdelim (&line, &siz, '\n', in);
        if (!(len == 4 && line && strcmp (line, "foo\n") == 0))
          { free (line); fclose (in); return 2; }
        free (line);
      }
      {
        /* Test result for a NULL buffer and a non-zero size.
           This crashes on FreeBSD 8.0.  */
        char *line = NULL;
        size_t siz = (size_t)(~0) / 4;
        if (getdelim (&line, &siz, '\n', in) == -1)
          { fclose (in); return 3; }
        free (line);
      }
      fclose (in);
      return 0;
    }
    ]])],
             [gl_cv_func_working_getdelim=yes],
             [gl_cv_func_working_getdelim=no],
             [dnl We're cross compiling.
              dnl Guess it works on glibc2 systems and musl systems.
              AC_EGREP_CPP([Lucky GNU user],
                [
#include <features.h>
#ifdef __GNU_LIBRARY__
 #if (__GLIBC__ >= 2) && !defined __UCLIBC__
  Lucky GNU user
 #endif
#endif
                ],
                [gl_cv_func_working_getdelim="guessing yes"],
                [case "$host_os" in
                   *-musl* | midipix*) gl_cv_func_working_getdelim="guessing yes" ;;
                   *)                  gl_cv_func_working_getdelim="$gl_cross_guess_normal" ;;
                 esac
                ])
             ])
           ;;
       esac
      ])
    case "$gl_cv_func_working_getdelim" in
      *yes) ;;
      *) REPLACE_GETDELIM=1 ;;
    esac
  else
    HAVE_GETDELIM=0
    case "$gl_cv_onwards_func_getdelim" in
      future*) REPLACE_GETDELIM=1 ;;
    esac
  fi

  if test $ac_cv_have_decl_getdelim = no; then
    HAVE_DECL_GETDELIM=0
  fi
])

# Prerequisites of lib/getdelim.c.
AC_DEFUN([gl_PREREQ_GETDELIM],
[
  AC_CHECK_FUNCS([flockfile funlockfile])
  AC_CHECK_DECLS([getc_unlocked])
])
