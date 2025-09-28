# assert-h.m4
dnl Copyright (C) 2011-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl From Paul Eggert.

AC_DEFUN([gl_ASSERT_H],
[
  AC_CACHE_CHECK([for static_assert], [gl_cv_static_assert],
    [gl_save_CFLAGS=$CFLAGS
     for gl_working in "yes, a keyword" "yes, an <assert.h> macro"; do
      AS_CASE([$gl_working],
        [*assert.h*], [CFLAGS="$gl_save_CFLAGS -DINCLUDE_ASSERT_H"])

      AC_COMPILE_IFELSE(
       [AC_LANG_PROGRAM(
          [[#if defined __clang__ && __STDC_VERSION__ < 202311
             #pragma clang diagnostic error "-Wc2x-extensions"
             #pragma clang diagnostic error "-Wc++1z-extensions"
            #endif
            #ifdef INCLUDE_ASSERT_H
             #include <assert.h>
            #endif
            static_assert (2 + 2 == 4, "arithmetic does not work");
            static_assert (2 + 2 == 4);
          ]],
          [[
            static_assert (sizeof (char) == 1, "sizeof does not work");
            static_assert (sizeof (char) == 1);
          ]])],
       [gl_cv_static_assert=$gl_working],
       [gl_cv_static_assert=no])
      CFLAGS=$gl_save_CFLAGS
      test "$gl_cv_static_assert" != no && break
     done])

  GL_GENERATE_ASSERT_H=false
  AS_CASE([$gl_cv_static_assert],
    [yes*keyword*],
      [AC_DEFINE([HAVE_C_STATIC_ASSERT], [1],
         [Define to 1 if the static_assert keyword works.])],
    [no],
      [GL_GENERATE_ASSERT_H=true
       gl_NEXT_HEADERS([assert.h])])

  dnl The "zz" puts this toward config.h's end, to avoid potential
  dnl collisions with other definitions.
  dnl #undef assert so that programs are not tempted to use it without
  dnl specifically including assert.h.
  dnl #undef __ASSERT_H__ so that on IRIX, when programs later include
  dnl <assert.h>, this include actually defines assert.
  dnl Break the #undef_s apart with a comment so that 'configure' does
  dnl not comment them out.
  AH_VERBATIM([zzstatic_assert],
[#if (!defined HAVE_C_STATIC_ASSERT && !defined assert \
     && (!defined __cplusplus \
         || (__cpp_static_assert < 201411 \
             && __GNUG__ < 6 && __clang_major__ < 6)))
 #include <assert.h>
 #undef/**/assert
 #ifdef __sgi
  #undef/**/__ASSERT_H__
 #endif
 /* Solaris 11.4 <assert.h> defines static_assert as a macro with 2 arguments.
    We need it also to be invocable with a single argument.  */
 #if defined __sun && (__STDC_VERSION__ - 0 >= 201112L) && !defined __cplusplus
  #undef/**/static_assert
  #define static_assert _Static_assert
 #endif
#endif])
])
