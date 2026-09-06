# stdalign.m4
# serial 3
dnl Copyright 2011-2026 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.
dnl This file is offered as-is, without any warranty.

# Check for alignas and alignof that conform to C23.

dnl Written by Paul Eggert and Bruno Haible.

# Prepare for substituting <stdalign.h> if it is not supported.

AC_DEFUN([gl_ALIGNASOF],
[
  AC_CACHE_CHECK([for alignas and alignof],
    [gl_cv_header_working_stdalign_h],
    [gl_saved_CFLAGS=$CFLAGS
     for gl_working in "yes, keywords" "yes, <stdalign.h> macros"; do
      AS_CASE([$gl_working],
        [*stdalign.h*], [CFLAGS="$gl_saved_CFLAGS -DINCLUDE_STDALIGN_H"])
      AC_COMPILE_IFELSE(
       [AC_LANG_PROGRAM(
          [[#include <stdint.h>
            #ifdef INCLUDE_STDALIGN_H
             #include <stdalign.h>
            #endif
            #include <stddef.h>

            /* Test that alignof yields a result consistent with offsetof.
               This catches GCC bug 52023
               <https://gcc.gnu.org/PR52023>.  */
            #ifdef __cplusplus
               template <class t> struct alignof_helper { char a; t b; };
            # define ao(type) offsetof (alignof_helper<type>, b)
            #else
            # define ao(type) offsetof (struct { char a; type b; }, b)
            #endif
            char test_double[ao (double) % _Alignof (double) == 0 ? 1 : -1];
            char test_long[ao (long int) % _Alignof (long int) == 0 ? 1 : -1];
            char test_alignof[alignof (double) == _Alignof (double) ? 1 : -1];

            /* Test alignas only on platforms where gnulib can help.  */
            #if \
                ((defined __cplusplus && 201103 <= __cplusplus) \
                 || (__TINYC__ && defined __attribute__) \
                 || (defined __APPLE__ && defined __MACH__ \
                     ? 4 < __GNUC__ + (1 <= __GNUC_MINOR__) \
                     : __GNUC__) \
                 || (__ia64 && (61200 <= __HP_cc || 61200 <= __HP_aCC)) \
                 || __ICC || 0x590 <= __SUNPRO_C || 0x0600 <= __xlC__ \
                 || 1300 <= _MSC_VER)
              struct alignas_test { char c; char alignas (8) alignas_8; };
              char test_alignas[offsetof (struct alignas_test, alignas_8) == 8
                                ? 1 : -1];
            #endif
          ]])],
       [gl_cv_header_working_stdalign_h=$gl_working],
       [gl_cv_header_working_stdalign_h=no])

      CFLAGS=$gl_saved_CFLAGS
      test "$gl_cv_header_working_stdalign_h" != no && break
     done])

  AS_CASE([$gl_cv_header_working_stdalign_h],
    [yes*keyword*],
      [AC_DEFINE([HAVE_C_ALIGNASOF], [1],
         [Define to 1 if the alignas and alignof keywords work.])])

  dnl The "zz" puts this toward config.h's end, to avoid potential
  dnl collisions with other definitions.
  AH_VERBATIM([zzalignas],
[#if !defined HAVE_C_ALIGNASOF \
    && !(defined __cplusplus && 201103 <= __cplusplus) \
    && !defined alignof
# if defined HAVE_STDALIGN_H
#  include <stdalign.h>
# endif

/* ISO C23 alignas and alignof for platforms that lack it.

   References:
   ISO C23 (latest free draft
   <https://www.open-std.org/jtc1/sc22/wg14/www/docs/n3220.pdf>)
   sections 6.2.8, 6.7.6.
   C++11 (latest free draft
   <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2011/n3242.pdf>)
   section 18.10. */

/* alignof (TYPE), also known as _Alignof (TYPE), yields the alignment
   requirement of a structure member (i.e., slot or field) that is of
   type TYPE, as an integer constant expression.

   This differs from GCC's and clang's __alignof__ operator, which can
   yield a better-performing alignment for an object of that type.  For
   example, on x86 with GCC and on Linux/x86 with clang,
   __alignof__ (double) and __alignof__ (long long) are 8, whereas
   alignof (double) and alignof (long long) are 4 unless the option
   '-malign-double' is used.

   The result cannot be used as a value for an 'enum' constant, if you
   want to be portable to HP-UX 10.20 cc and AIX 3.2.5 xlc.  */

/* GCC releases before GCC 4.9 had a bug in _Alignof.  See GCC bug 52023
   <https://gcc.gnu.org/PR52023>.
   clang versions < 8.0.0 have the same bug.
   IBM XL C V16.1.0 cc (non-clang) has the same bug.  */
#  if (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112 \
       || (defined __GNUC__ && __GNUC__ < 4 + (__GNUC_MINOR__ < 9) \
           && !defined __clang__) \
       || (defined __clang__ && __clang_major__ < 8) \
       || defined __xlC__)
#   undef/**/_Alignof
#   ifdef __cplusplus
#    if (201103 <= __cplusplus || defined _MSC_VER)
#     define _Alignof(type) alignof (type)
#    else
      template <class __t> struct __alignof_helper { char __a; __t __b; };
#     if (defined __GNUC__ && 4 <= __GNUC__) || defined __clang__
#      define _Alignof(type) __builtin_offsetof (__alignof_helper<type>, __b)
#     else
#      define _Alignof(type) offsetof (__alignof_helper<type>, __b)
#     endif
#     define _GL_STDALIGN_NEEDS_STDDEF 1
#    endif
#   else
#    if (defined __GNUC__ && 4 <= __GNUC__) || defined __clang__
#     define _Alignof(type) __builtin_offsetof (struct { char __a; type __b; }, __b)
#    else
#     define _Alignof(type) offsetof (struct { char __a; type __b; }, __b)
#     define _GL_STDALIGN_NEEDS_STDDEF 1
#    endif
#   endif
#  endif
#  if ! (defined __cplusplus && (201103 <= __cplusplus || defined _MSC_VER))
#   undef/**/alignof
#   define alignof _Alignof
#  endif

/* alignas (A), also known as _Alignas (A), aligns a variable or type
   to the alignment A, where A is an integer constant expression.  For
   example:

      int alignas (8) foo;
      struct s { int a; int alignas (8) bar; };

   aligns the address of FOO and the offset of BAR to be multiples of 8.

   A should be a power of two that is at least the type's alignment
   and at most the implementation's alignment limit.  This limit is
   2**28 on typical GNUish hosts, and 2**13 on MSVC.  To be portable
   to MSVC through at least version 10.0, A should be an integer
   constant, as MSVC does not support expressions such as 1 << 3.
   To be portable to Sun C 5.11, do not align auto variables to
   anything stricter than their default alignment.

   The following C23 requirements are not supported here:

     - If A is zero, alignas has no effect.
     - alignas can be used multiple times; the strictest one wins.
     - alignas (TYPE) is equivalent to alignas (alignof (TYPE)).

   */
# if !defined __STDC_VERSION__ || __STDC_VERSION__ < 201112
#  if defined __cplusplus && (201103 <= __cplusplus || defined _MSC_VER)
#   define _Alignas(a) alignas (a)
#  elif (!defined __attribute__ \
         && ((defined __APPLE__ && defined __MACH__ \
              ? 4 < __GNUC__ + (1 <= __GNUC_MINOR__) \
              : __GNUC__ && !defined __ibmxl__) \
             || (4 <= __clang_major__) \
             || (__ia64 && (61200 <= __HP_cc || 61200 <= __HP_aCC)) \
             || __ICC || 0x590 <= __SUNPRO_C || 0x0600 <= __xlC__))
#   define _Alignas(a) __attribute__ ((__aligned__ (a)))
#  elif 1300 <= _MSC_VER
#   define _Alignas(a) __declspec (align (a))
#  endif
# endif
# if !defined HAVE_STDALIGN_H
#  if ((defined _Alignas \
        && !(defined __cplusplus \
             && (201103 <= __cplusplus || defined _MSC_VER))) \
       || (defined __STDC_VERSION__ && 201112 <= __STDC_VERSION__ \
           && !defined __xlC__))
#   define alignas _Alignas
#  endif
# endif

# if defined _GL_STDALIGN_NEEDS_STDDEF
#  include <stddef.h>
# endif
#endif])
])

AC_DEFUN([gl_STDALIGN_H],
[
  AC_REQUIRE([gl_ALIGNASOF])
  if test "$gl_cv_header_working_stdalign_h" = no; then
    GL_GENERATE_STDALIGN_H=true
  else
    GL_GENERATE_STDALIGN_H=false
  fi

  gl_CHECK_NEXT_HEADERS([stdalign.h])
  if test $ac_cv_header_stdalign_h = yes; then
    HAVE_STDALIGN_H=1
  else
    HAVE_STDALIGN_H=0
  fi
  AC_SUBST([HAVE_STDALIGN_H])
])
