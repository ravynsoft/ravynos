# serial 22

dnl From Jim Meyering.
dnl
dnl Check whether struct dirent has a member named d_ino.
dnl

# Copyright (C) 1997, 1999-2001, 2003-2004, 2006-2007, 2009-2023 Free Software
# Foundation, Inc.

# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_CHECK_TYPE_STRUCT_DIRENT_D_INO],
  [AC_REQUIRE([AC_CANONICAL_HOST]) dnl for cross-compiles
   AC_CACHE_CHECK([for d_ino member in directory struct],
                  [gl_cv_struct_dirent_d_ino],
     [AC_RUN_IFELSE(
        [AC_LANG_PROGRAM(
           [[#include <sys/types.h>
             #include <sys/stat.h>
             #include <dirent.h>
           ]],
           [[DIR *dp = opendir (".");
             struct dirent *e;
             struct stat st;
             if (! dp)
               return 1;
             e = readdir (dp);
             if (! e)
               { closedir (dp); return 2; }
             if (lstat (e->d_name, &st) != 0)
               { closedir (dp); return 3; }
             if (e->d_ino != st.st_ino)
               { closedir (dp); return 4; }
             closedir (dp);
             return 0;
           ]])],
           [gl_cv_struct_dirent_d_ino=yes],
           [gl_cv_struct_dirent_d_ino=no],
           [case "$host_os" in
                                 # Guess yes on glibc systems with Linux kernel.
              linux*-gnu*)       gl_cv_struct_dirent_d_ino="guessing yes" ;;
                                 # Guess yes on musl systems with Linux kernel.
              linux*-musl*)      gl_cv_struct_dirent_d_ino="guessing yes" ;;
                                 # Guess yes on systems that emulate the Linux system calls.
              midipix*)          gl_cv_struct_dirent_d_ino="guessing yes" ;;
                                 # Guess no on native Windows.
              mingw* | windows*) gl_cv_struct_dirent_d_ino="guessing no" ;;
                                 # If we don't know, obey --enable-cross-guesses.
              *)                 gl_cv_struct_dirent_d_ino="$gl_cross_guess_normal" ;;
            esac
           ])])
   case "$gl_cv_struct_dirent_d_ino" in
     *yes)
       AC_DEFINE([D_INO_IN_DIRENT], [1],
         [Define if struct dirent has a member d_ino that actually works.])
       ;;
   esac
  ]
)
