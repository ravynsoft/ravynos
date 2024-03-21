# mathfunc.m4 serial 17
dnl Copyright (C) 2010-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

# gl_MATHFUNC(FUNC, RETTYPE, PARAMTYPES [, INCLUDES] [, EXTRA-CODE])
# ------------------------------------------------------------------
# tests whether the function FUNC is available in libc or libm.
# RETTYPE is the return type. PARAMTYPES is a parameter list, with parentheses.
# It sets FUNC_LIBM to empty or "-lm" accordingly.

AC_DEFUN([gl_MATHFUNC],
[
  dnl We need the RETTYPE and PARAMTYPES in order to force linking with the
  dnl function.
  dnl 1) With gcc >= 4.3 on glibc/x86_64, calls to the 'fabs' function
  dnl    are inlined by the compiler, therefore linking of these calls does
  dnl    not require -lm, but taking the function pointer of 'fabs' does.
  dnl 2) On MSVC 9, many math functions exist only as macros with arguments,
  dnl    whereas the function pointer is undefined.
  dnl On the other hand, taking just the function pointer is not enough.
  dnl 1) On AIX 7.1, when 'long double' is 128 bit large ("xlc -qldbl128" or
  dnl    "xlc -qlongdouble" or "gcc -mlong-double-128") many math functions
  dnl    exist as macros with arguments, that may reference libm or even
  dnl    completely undefined functions such as __rint128.
  dnl 2) In AIX 7.1 with gcc 4.2, when optimization is turned on, calls to
  dnl    rint() with simple arguments are turned into rintf() calls by the
  dnl    compiler. But while rint() is resides in libc, rintf() is in libm.
  m4_pushdef([func], [$1])
  m4_pushdef([FUNC], [m4_translit([$1],[abcdefghijklmnopqrstuvwxyz],
                                       [ABCDEFGHIJKLMNOPQRSTUVWXYZ])])
  m4_pushdef([ARGS], [m4_bpatsubst(
                        [m4_bpatsubst(
                           [m4_bpatsubst(
                              [m4_bpatsubst(
                                 [m4_bpatsubst(
                                    [m4_bpatsubst(
                                       [m4_bpatsubst(
                                          [m4_bpatsubst(
                                             [m4_bpatsubst(
                                                [m4_bpatsubst(
                                                   [m4_bpatsubst(
                                                      [m4_bpatsubst(
                                                         [m4_bpatsubst(
                                                            [$3],
                                                            [fenv_t\( const\)? \*], [&fenv_ret])],
                                                         [fexcept_t\( const\)? \*], [&fx_ret])],
                                                      [int\( const\)? \*],
                                                      [&i_ret])],
                                                   [float\( const\)? \*], [&f_ret])],
                                                [double\( const\)? \*], [&d_ret])],
                                             [long double\( const\)? \*], [&l_ret])],
                                          [fp_rnd], [1])],
                                       [fp_except_t], [1])],
                                    [int], [2])],
                                 [float], [1.618034f])],
                              [long double], [1.618033988749894848L])],
                           [double], [1.6180339887])],
                        [void], [])])
  FUNC[]_LIBM=
  AC_CACHE_CHECK([whether func() can be used without linking with libm],
    [gl_cv_func_]func[_no_libm],
    [
      AC_LINK_IFELSE(
        [AC_LANG_PROGRAM(
           [[#ifndef __NO_MATH_INLINES
             # define __NO_MATH_INLINES 1 /* for glibc */
             #endif
             #include <math.h>
             $4
             $2 (* volatile funcptr) $3 = ]func[;
             int i_ret;
             float f_ret;
             double d_ret;
             long double l_ret;]],
           [[$2 y = funcptr ]ARGS[ + ]func[ ]ARGS[;
             $5
             return y < 0.3 || y > 1.7;
           ]])],
        [gl_cv_func_]func[_no_libm=yes],
        [gl_cv_func_]func[_no_libm=no])
    ])
  if test $gl_cv_func_[]func[]_no_libm = no; then
    AC_CACHE_CHECK([whether func() can be used with libm],
      [gl_cv_func_]func[_in_libm],
      [
        save_LIBS="$LIBS"
        LIBS="$LIBS -lm"
        AC_LINK_IFELSE(
          [AC_LANG_PROGRAM(
             [[#ifndef __NO_MATH_INLINES
               # define __NO_MATH_INLINES 1 /* for glibc */
               #endif
               #include <math.h>
               $4
               $2 (* volatile funcptr) $3 = ]func[;
               int i_ret;
               float f_ret;
               double d_ret;
               long double l_ret;]],
             [[$2 y = funcptr ]ARGS[ + ]func[ ]ARGS[;
               $5
               return y < 0.3 || y > 1.7;
             ]])],
          [gl_cv_func_]func[_in_libm=yes],
          [gl_cv_func_]func[_in_libm=no])
        LIBS="$save_LIBS"
      ])
    if test $gl_cv_func_[]func[]_in_libm = yes; then
      FUNC[]_LIBM=-lm
    fi
  fi
  AC_SUBST(FUNC[_LIBM])
  m4_popdef([ARGS])
  m4_popdef([FUNC])
  m4_popdef([func])
])

# gl_COMMON_DOUBLE_MATHFUNC(FUNC)
# -------------------------------
# tests whether the function FUNC is available in libc or libm.
# It sets FUNC_LIBM to empty or "-lm" accordingly.
# FUNC must be one of the following functions, that are present on all systems
# and provided by libm on all systems except Mac OS X, BeOS, Haiku:
#   acos asin atan atan2 cbrt cos cosh erf erfc exp fmod hypot j0 j1 jn lgamma
#   log log10 log1p pow remainder sin sinh sqrt tan tanh y0 y1 yn

AC_DEFUN([gl_COMMON_DOUBLE_MATHFUNC],
[
  AC_REQUIRE([gl_COMMON_DOUBLE_MATHFUNC_TEST])
  m4_pushdef([FUNC], [m4_translit([$1],[abcdefghijklmnopqrstuvwxyz],
                                       [ABCDEFGHIJKLMNOPQRSTUVWXYZ])])
  FUNC[]_LIBM="$POW_LIBM"
  AC_SUBST(FUNC[_LIBM])
  m4_popdef([FUNC])
])

AC_DEFUN([gl_COMMON_DOUBLE_MATHFUNC_TEST],
[
  dnl We could use any of the following:
  dnl gl_MATHFUNC([acos], [double], [(double)])
  dnl gl_MATHFUNC([asin], [double], [(double)])
  dnl gl_MATHFUNC([atan], [double], [(double)])
  dnl gl_MATHFUNC([atan2], [double], [(double, double)])
  dnl gl_MATHFUNC([cbrt], [double], [(double)])
  dnl gl_MATHFUNC([cos], [double], [(double)])
  dnl gl_MATHFUNC([cosh], [double], [(double)])
  dnl gl_MATHFUNC([erf], [double], [(double)])
  dnl gl_MATHFUNC([erfc], [double], [(double)])
  dnl gl_MATHFUNC([exp], [double], [(double)])
  dnl gl_MATHFUNC([fmod], [double], [(double, double)])
  dnl gl_MATHFUNC([hypot], [double], [(double, double)])
  dnl gl_MATHFUNC([j0], [double], [(double)])
  dnl gl_MATHFUNC([j1], [double], [(double)])
  dnl gl_MATHFUNC([jn], [double], [(int, double)])
  dnl gl_MATHFUNC([lgamma], [double], [(double)])
  dnl gl_MATHFUNC([log], [double], [(double)])
  dnl gl_MATHFUNC([log10], [double], [(double)])
  dnl gl_MATHFUNC([log1p], [double], [(double)])
  dnl gl_MATHFUNC([pow], [double], [(double, double)])
  dnl gl_MATHFUNC([remainder], [double], [(double, double)])
  dnl gl_MATHFUNC([sin], [double], [(double)])
  dnl gl_MATHFUNC([sinh], [double], [(double)])
  dnl gl_MATHFUNC([sqrt], [double], [(double)])
  dnl gl_MATHFUNC([tan], [double], [(double)])
  dnl gl_MATHFUNC([tanh], [double], [(double)])
  dnl gl_MATHFUNC([y0], [double], [(double)])
  dnl gl_MATHFUNC([y1], [double], [(double)])
  dnl gl_MATHFUNC([yn], [double], [(int, double)])
  gl_MATHFUNC([pow], [double], [(double, double)])
])
